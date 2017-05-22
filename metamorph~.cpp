#include "maxcpp6.h"
#include <string>
#include <metamorph/metamorph.h>

// inherit from the MSP base class, template-specialized for myself:

using namespace metamorph;

class Metamorph : public MspCpp6<Metamorph> {
public:
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
    
    Metamorph(t_symbol * sym, long ac, t_atom * av) {
        setupIO(4, 1);
        
        
        hops_per_frame = 4;
        
        fx = new FX();
        
        fx->hop_size(sys_getblksize());
        fx->frame_size(sys_getblksize() * hops_per_frame);
        
        transpose = new Transposition();
        fx->add_transformation(transpose);
        
        hdist = new HarmonicDistortion();
        fx->add_transformation(hdist);
        
        preserve_transients = true;
        preserve_envelope = false;
        
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
        
        
        for(int i=0; i<sampleframes; i++)
            out[i] = 0.0;
        
        fx->harmonic_scale(harmonic_scale);
        fx->residual_scale(residual_scale);
        fx->transient_scale(transient_scale);
        
        fx->preserve_transients(preserve_transients);
        
        transpose->transposition(0.0);
        fx->preserve_envelope(preserve_envelope);
        
        hdist->harmonic_distortion(1.0);
        hdist->fundamental_frequency(0.0);
        
        fx->process_frame(sampleframes, in, sampleframes, out);
    }
    
    //	// optional method: gets called when the dsp chain is modified
    //	// if not given, the MspCpp will use Metamorph::perform by default
    void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
        // specify a perform method manually:
        fx->reset();
        fx->hop_size(sys_getblksize());
        fx->frame_size(sys_getblksize() * hops_per_frame);
        
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
