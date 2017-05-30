#include "maxcpp6.h"
#include <string>
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

class Metamorph : public MspCpp6<Metamorph> {
public:
    RingBuffer inBuffer, outBuffer;
    std::vector<double> inTempFrame;
    std::vector<double> outTempFrame;
    
    int hops_per_frame;
    FX* fx;
    Transposition* transpose;
    HarmonicDistortion* hdist;
    
    double harmonic_scale,
    residual_scale,
    transient_scale,
    preserve_transients,
    transposition_factor,
    preserve_envelope,
    harmonic_distortion,
    fundamental_frequency;
    
    Metamorph(t_symbol * sym, long ac, t_atom * av) : inBuffer(sys_getsr()), outBuffer(sys_getsr()) {
        setupIO(4, 1);
        
        hops_per_frame = 4;
        
        fx = new FX();
        
        fx->hop_size(512);
        fx->frame_size(fx->hop_size() * hops_per_frame);
        
        transpose = new Transposition();
        fx->add_transformation(transpose);
        
        hdist = new HarmonicDistortion();
        fx->add_transformation(hdist);
        
        preserve_transients = true;
        preserve_envelope = false;
        
        inTempFrame.assign(512, 0.0);
        outTempFrame.assign(512, 0.0);
        
        post("object created");
    }
    
    ~Metamorph() {
        delete fx;
        delete transpose;
        post("object freed");
        
        
    }
    
    // methods:
//    void bang(long inlet) {
//        post("bang in inlet %i!", inlet);
//    }
//    void test(long inlet, t_symbol * s, long ac, t_atom * av) {
//        post("%s in inlet %i (%i args)", s->s_name, inlet, ac);
//    }
    
    void handleFloat(long inlet, double v) {
        std::string param = "";
    
        switch (inlet) {
            case 1:
                param = "Harmonic Scale: ";
                harmonic_scale = v;
                break;
            case 2:
                param = "Residual Scale: ";
                residual_scale = v;
                break;
            case 3:
                param = "Transient Scale: ";
                transient_scale = v;
            default:
                break;
        }
        
        post("%s %f", param.c_str(), inlet, v);
    }
    
    
    // default signal processing method is called 'perform'
    void perform(double **ins, long numins, double **outs, long numouts, long sampleframes) {
        double* in = ins[0];
        double* out = outs[0];
        
        memset(out, 0, sizeof(double) * sampleframes);
        
        inBuffer.write(sampleframes, in);
        
        if(inBuffer.fillCount >= fx->hop_size())
        {
            fill(inTempFrame.begin(), inTempFrame.end(), 0.0);
            fill(outTempFrame.begin(), outTempFrame.end(), 0.0);
        
            fx->harmonic_scale(harmonic_scale);
            fx->residual_scale(residual_scale);
            fx->transient_scale(transient_scale);
            
            fx->preserve_transients(preserve_transients);
            
            transpose->transposition(0.0);
            fx->preserve_envelope(preserve_envelope);
            
            hdist->harmonic_distortion(1.0);
            hdist->fundamental_frequency(0.0);
        
            inBuffer.read(fx->hop_size(), &inTempFrame[0]);
            fx->process_frame(fx->hop_size(), &inTempFrame[0], fx->hop_size(), &outTempFrame[0]);
            outBuffer.write(fx->hop_size(), &outTempFrame[0]);
        }
        
        outBuffer.read(sampleframes, out);
    }
    
    //	// optional method: gets called when the dsp chain is modified
    //	// if not given, the MspCpp will use Metamorph::perform by default
    void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
        // specify a perform method manually:
        fx->reset();
        
//        fx->hop_size(512);
//        fx->frame_size(fx->hop_size() * hops_per_frame);
        
        REGISTER_PERFORM(Metamorph, perform);
    }
};

C74_EXPORT int main(void) {
    // create a class with the given name:
    Metamorph::makeMaxClass("metamorph~");
//    REGISTER_METHOD(Metamorph, bang);
//    REGISTER_METHOD_GIMME(Metamorph, test);
    
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
    REGISTER_INLET_FLOAT(Metamorph, handleFloat);
}
