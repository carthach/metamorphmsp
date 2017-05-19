#include "maxcpp6.h"
#include <metamorph/metamorph.h>

// inherit from the MSP base class, template-specialized for myself:

using namespace metamorph;

class Example : public MspCpp6<Example> {
public:
    FX metamorph;
    
    Example(t_symbol * sym, long ac, t_atom * av) {
        setupIO(3, 1);
        post("object created");
        
        
        
    }
    
    ~Example() {
        post("object freed");
        
        
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
        //		// example code to invert inputs
        std::vector<sample> inBuffer(sampleframes);
        std::vector<sample> outBuffer(sampleframes);
        
        double* in = ins[0];
        double* out = outs[0];
        
        for(int i=0; i<sampleframes; i++)
        {
            inBuffer[i] = in[i];
        }
        
        metamorph.process_frame(sampleframes, &inBuffer[0], sampleframes, &outBuffer[0]);
        
        for(int i=0; i<sampleframes; i++)
        {
            out[i] = outBuffer[i];
        }
    }
    
    //	// optional method: gets called when the dsp chain is modified
    //	// if not given, the MspCpp will use Example::perform by default
    void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
        // specify a perform method manually:
        
        metamorph.hop_size(sys_getblksize());
        metamorph.frame_size(metamorph.hop_size() * 4);
        metamorph.reset();
        
        REGISTER_PERFORM(Example, perform);
    }
};

C74_EXPORT int main(void) {
    // create a class with the given name:
    Example::makeMaxClass("metamorph~");
    REGISTER_METHOD(Example, bang);
    REGISTER_METHOD_GIMME(Example, test);
    
    REGISTER_INLET_FLOAT(Example, handleFloat);
    REGISTER_INLET_FLOAT(Example, handleFloat);
    REGISTER_INLET_FLOAT(Example, handleFloat);
}
