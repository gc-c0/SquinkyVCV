#pragma once

#include "engine/Module.hpp"
#include "engine/Port.hpp"
#include "engine/Engine.hpp"

using Input = rack::engine::Input;
using Output = rack::engine::Output;
using Param = rack::engine::Param;
using Light = rack::engine::Light;

/**
 * Base class for composites embeddable in a VCV Widget
 * This is used for "real" implementations
 */
class WidgetComposite
{
public:
    WidgetComposite(rack::engine::Module * parent) :
        inputs(parent->inputs),
        outputs(parent->outputs),
        params(parent->params),
        lights(parent->lights)
    {
    }
    virtual void step()
    {
    };
    float engineGetSampleRate()
    {
#ifdef __V1
        return APP->engine->getSampleRate();
#else  
        return ::engineGetSampleRate();
#endif
    }
    
    float engineGetSampleTime()
    {
#ifdef __V1
        return APP->engine->getSampleTime();
#else  
        return ::engineGetSampleTime();
#endif
    }
protected:
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
    std::vector<Light>& lights;
private:
};
