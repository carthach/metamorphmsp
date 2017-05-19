#include "maxcpp6.h"
#include <metamorph/metamorph.h>

// inherit from the MSP base class, template-specialized for myself:

using namespace metamorph;

class RingBuffer {
public:
    std::vector<double> buffer;
    
    int readIndex;
    int writeIndex;
    int fillCount;
    
    RingBuffer(int size)
    {
        buffer.assign(size, 0.0);
        readIndex = 0;
        writeIndex = 0;
        fillCount = 0;
    }
    
    int write(int noOfSamples, const double* data)
    {
        if(fillCount >= buffer.size())
            return 0;
        
        for(int i=0; i<noOfSamples; i++)
        {
            buffer[writeIndex] = data[i];
            
            if(writeIndex == buffer.size()-1)
                writeIndex = 0;
            else
                writeIndex++;
        }
        
        this->fillCount += noOfSamples;
        
        return noOfSamples;
    }
    
    int read(int noOfSamples, double* data)
    {
        if(readIndex == writeIndex)
            return 0;
        
        for(int i=0; i<noOfSamples; i++)
        {
            data[i] = buffer[readIndex];
            
            if(readIndex == buffer.size()-1)
                readIndex = 0;
            else
                readIndex++;
        }
        
        this->fillCount -= noOfSamples;
        
        return noOfSamples;
    }
};

void call_from_thread() {
    std::cout << "Hello, World" << std::endl;
}

class Metamorph : public MspCpp6<Metamorph> {
public:
    FX metamorph;
    
    
    
    RingBuffer inputRingBuffer, outputRingBuffer;
    std::vector<double> tempInputBuffer, tempOutputBuffer;
    
    int hopSize = 512;
    int frameSize = hopSize * 4;
    
    Metamorph(t_symbol * sym, long ac, t_atom * av) : inputRingBuffer(1024), outputRingBuffer(1024) {
        setupIO(3, 1);
        
        setupBuffers();
        
        metamorph.harmonic_scale(1.0);
        metamorph.residual_scale(1.0);
        metamorph.transient_scale(1.0);
        
        post("object created");
    }
    
    ~Metamorph() {
        post("object freed");
    }
    
    void setupBuffers()
    {
        metamorph.hop_size(hopSize);
        metamorph.frame_size(frameSize);
        metamorph.reset();
        
        tempInputBuffer.assign(hopSize, 0.0);
        tempOutputBuffer.assign(hopSize, 0.0);
    }
    
    // methods:
    void bang(long inlet) {
        post("bang in inlet %i!", inlet);
    }
    void test(long inlet, t_symbol * s, long ac, t_atom * av) {
        post("%s in inlet %i (%i args)", s->s_name, inlet, ac);
    }
    
    void handleFloat(long inlet, double v) {
        post("inlet %ld float %f", inlet, v);
        //        outlet_float(m_outlets[0], v);
        
        switch (inlet) {
            case 0:
                metamorph.harmonic_scale(v);
                break;
            case 1:
                metamorph.residual_scale(v);
                break;
            case 2:
                metamorph.transient_scale(v);
            default:
                break;
        }
        
    }
    
    // default signal processing method is called 'perform'
    void perform(double **ins, long numins, double **outs, long numouts, long sampleframes) {
        // example code to invert inputs
        
        //Just analyse mono
        numouts = 1;
        
        for (long channel = 0; channel < numouts; channel++) {
            double * in = ins[channel];
            double * out = outs[channel];
            
            inputRingBuffer.write(sampleframes, in);
            
            for(int i=0; i<sampleframes; i++)
                out[i] = 0.0;
            
            if(inputRingBuffer.fillCount >= hopSize)
            {
                inputRingBuffer.read(hopSize, &tempInputBuffer[0]);
                metamorph.process_frame(hopSize, &tempInputBuffer[0], hopSize, &tempOutputBuffer[0]);
                outputRingBuffer.write(hopSize, &tempOutputBuffer[0]);
            }
            
            int samplesProcessed = outputRingBuffer.read(sampleframes, out);
        }
    }
    
    // optional method: gets called when the dsp chain is modified
    // if not given, the MspCpp will use Example::perform by default
    void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
        // specify a perform method manually:
        setupBuffers();
        
        REGISTER_PERFORM(Metamorph, perform);
    }
};

C74_EXPORT int main(void) {
    // create a class with the given name:
    Metamorph::makeMaxClass("metamorph~");
    REGISTER_METHOD(Metamorph, bang);
    REGISTER_METHOD_GIMME(Metamorph, test);
    
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
}
