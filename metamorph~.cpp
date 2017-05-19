#include "maxcpp6.h"
#include <string>
#include <metamorph/metamorph.h>

// inherit from the MSP base class, template-specialized for myself:

using namespace metamorph;

class Metamorph : public MspCpp6<Metamorph> {
public:
    FX metamorph;
    
    Metamorph(t_symbol * sym, long ac, t_atom * av) {
        setupIO(3, 1);
        post("object created");
        
        metamorph.preserve_transients(true);
        metamorph.preserve_envelope(true);
    }
    
    ~Metamorph() {
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
            case 0:
                param = "Harmonic Scale: ";
                metamorph.harmonic_scale(v);
                break;
            case 1:
                param = "Residual Scale: ";
                metamorph.residual_scale(v);
                break;
            case 2:
                param = "Transient Scale: ";
                metamorph.transient_scale(v);
            default:
                break;
        }
        
        post("%s %f", param.c_str(), inlet, v);
    }
    
    
    // default signal processing method is called 'perform'
    void perform(double **ins, long numins, double **outs, long numouts, long sampleframes) {
        double* in = ins[0];
        double* out = outs[0];
        
        metamorph.process_frame(sampleframes, in, sampleframes, out);
    }
    
    //	// optional method: gets called when the dsp chain is modified
    //	// if not given, the MspCpp will use Metamorph::perform by default
    void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
        // specify a perform method manually:
        metamorph.reset();
        
        metamorph.hop_size(maxvectorsize);
        metamorph.frame_size(maxvectorsize*4);
        
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
