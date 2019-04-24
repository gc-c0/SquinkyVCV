

#include "../Squinky.hpp"
#include "SqGfx.h"
#include "WidgetComposite.h"

#ifdef __V1
#include "widget/Widget.hpp"
#include "app.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#include "window.hpp"
#endif

#include "NoteDragger.h"
#include "NoteDisplay.h"

#include "UIPrefs.h"



NoteDragger::NoteDragger(MidiSequencerPtr seq, float initX, float initY) :
    sequencer(seq),
    startX(initX),
    startY(initY)
{ 
    curMousePositionX = initX;
    curMousePositionY = initY;
}

NoteDragger::~NoteDragger()
{
}

void NoteDragger::onDrag(float deltaX, float deltaY)
{
    curMousePositionX += deltaX;    
    curMousePositionY += deltaY;
    if (deltaX!=0 || deltaY != 0) {
      //  printf("*** dragger::onDragx=%.2f y=%.2f\n", deltaX, deltaY); fflush(stdout);
    }
       // printf("dragger::onDragx=%.2f y=%.2f\n", curMousePositionX, curMousePositionY); fflush(stdout);
} 


//******************************************************************
//extern int bnd_font;

NotePitchDragger::NotePitchDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y)
{
}

void NotePitchDragger::commit()
{
    printf("NotePitchDragger::commit\n"); fflush(stdout);
      auto scaler = sequencer->context->getScaler();
    assert(scaler);
    float verticalShift =  curMousePositionY - startY;
   // printf("vertical pix = %f\n", verticalShift); fflush(stdout);
    float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);

    // std::pair<int, int> x1 = PitchUtils::cvToPitch(transposeCV);
    // printf("quantized shift = %d, %d\n", x1.first, x1.second); fflush(stdout);
    int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);
    printf("will shift by %d semis\n", semiShift);
    printf("selection has %d notes\n", sequencer->selection->size());
     fflush(stdout);
    sequencer->editor->changePitch(semiShift);
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    SqGfx::drawText(vg, curMousePositionX, curMousePositionY, "mouse");
    drawNotes(vg);
}

void NotePitchDragger::drawNotes(NVGcontext *vg)
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

    float verticalShift =  curMousePositionY - startY;
   // printf("vertical pix = %f\n", verticalShift); fflush(stdout);
   // float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);

    //std::pair<int, int> x1 = PitchUtils::cvToPitch(transposeCV);
    //float quantizedCV = PitchUtils::pitchToCV(x1.first, x1.second);
    //printf("transpose cv = %f quant=%d\n", transposeCV, quantizedCV);

    // This was lifted from NoteDisplay.
    // Can we refactor and share?
    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    
    const int noteHeight = scaler->noteHeight();
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev);
        const float y = scaler->midiPitchToY(*ev) + verticalShift;
        const float width = scaler->midiTimeTodX(ev->duration);

        const bool selected = sequencer->selection->isSelected(ev);
        if (selected) {
          //  printf("drawing gdragged at %.2f, %.2f\n ", x, y);
            SqGfx::filledRect(
                vg,
                UIPrefs::DRAGGED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}
