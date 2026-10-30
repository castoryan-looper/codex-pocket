#pragma once
#include <stdint.h>

// Hardware-independent policy. Drivers translate encoder/touch events here;
// only returned intents may be submitted to the existing cloud action queue.
namespace pocket {
enum class RoundView { Choices, Confirm, Reader, Tools };
enum class RoundInput { Clockwise, CounterClockwise, TapA, TapB, TapCenter,
                        Back, SwipeNext, SwipePrevious, OpenDetails, OpenTools,
                        HoldMic, ReleaseMic, CancelMic };
enum class RoundIntent { None, SelectA, SelectB, OpenA, OpenB, Execute, NextPage, PreviousPage,
                         StartCapture, FinishCapture, CancelCapture };

struct RoundInteraction {
  RoundView view=RoundView::Choices;
  uint8_t selection=0;
  bool online=false, busy=false, approval=false, draft=false, recording=false;
  unsigned page=0, pages=1;
  bool ready() const { return online && !busy && !approval && !draft; }
  RoundIntent blocked() {
    const bool cancel=recording;recording=false;view=RoundView::Choices;page=0;
    return cancel ? RoundIntent::CancelCapture : RoundIntent::None;
  }
  RoundIntent input(RoundInput event) {
    // Release/cancel must always terminate a previously started capture,
    // including when network or permissions changed while a finger was down.
    if (event==RoundInput::ReleaseMic || event==RoundInput::CancelMic) {
      if (!recording) return RoundIntent::None;
      recording=false;
      return event==RoundInput::CancelMic || !online ? RoundIntent::CancelCapture : RoundIntent::FinishCapture;
    }
    if (event==RoundInput::Back) {
      if (recording) {recording=false;return RoundIntent::CancelCapture;}
      view=view==RoundView::Reader ? RoundView::Confirm : RoundView::Choices;
      page=0;return RoundIntent::None;
    }
    // Offline users must still be able to reach Wi-Fi/USB configuration.
    if (event==RoundInput::OpenTools && !busy && !approval && !draft && !recording) {
      view=RoundView::Tools;return RoundIntent::None;
    }
    if (event==RoundInput::HoldMic && online && !approval && !draft && !recording) {
      if (view==RoundView::Tools) return RoundIntent::None;
      recording=true; return RoundIntent::StartCapture;
    }
    if (!ready() || recording) return RoundIntent::None;
    if (event==RoundInput::OpenDetails && view==RoundView::Confirm) {
      view=RoundView::Reader;page=0;return RoundIntent::None;
    }
    if (view==RoundView::Reader) {
      if (event==RoundInput::Clockwise || event==RoundInput::SwipeNext) {
        if (pages && page<pages-1) {page++;return RoundIntent::NextPage;}
      } else if (event==RoundInput::CounterClockwise || event==RoundInput::SwipePrevious) {
        if (page) {page--;return RoundIntent::PreviousPage;}
      } else if (event==RoundInput::TapCenter) {view=RoundView::Confirm;page=0;}
      return RoundIntent::None;
    }
    if (view==RoundView::Choices) {
      if (event==RoundInput::TapA || event==RoundInput::TapB) {
        selection=event==RoundInput::TapB;view=RoundView::Confirm;
        return selection ? RoundIntent::OpenB : RoundIntent::OpenA;
      }
      if (event==RoundInput::Clockwise || event==RoundInput::CounterClockwise) {
        selection=1-selection;return selection ? RoundIntent::SelectB : RoundIntent::SelectA;
      }
      if (event==RoundInput::TapCenter) { view=RoundView::Confirm;return selection ? RoundIntent::OpenB : RoundIntent::OpenA; }
    } else if (view==RoundView::Confirm) {
      if (event==RoundInput::TapCenter) {
        // Lock immediately; duplicate touches cannot submit twice while waiting
        // for the server to acknowledge the new task. Reconcile on fresh state.
        busy=true;view=RoundView::Choices;return RoundIntent::Execute;
      }
      if (event==RoundInput::Clockwise || event==RoundInput::CounterClockwise) {
        view=RoundView::Choices;selection=1-selection;
        return selection ? RoundIntent::SelectB : RoundIntent::SelectA;
      }
    }
    return RoundIntent::None;
  }
};

// Circular hit regions avoid invisible corner buttons and tap/scroll confusion.
enum class RoundHit { None, A, B, Center, Back, Details, Mic, Tools };
inline RoundHit roundHit(int x,int y,int diameter,RoundView view) {
  if (diameter<200 || x<0 || y<0 || x>=diameter || y>=diameter) return RoundHit::None;
  const int dx=2*x-diameter,dy=2*y-diameter;
  if (int64_t(dx)*dx+int64_t(dy)*dy>int64_t(diameter-12)*(diameter-12)) return RoundHit::None;
  const int px=x*360/diameter,py=y*360/diameter;
  if (py>=270 && py<=316 && px>=124 && px<=236) return RoundHit::Mic;
  if (py>=270 && px<105) return RoundHit::Back;
  if (py>=270 && px>255) return RoundHit::Tools;
  if (view==RoundView::Choices && px>=54 && px<=306) {
    if (py>=112 && py<179) return RoundHit::A;
    if (py>=187 && py<254) return RoundHit::B;
  }
  if (view==RoundView::Confirm && py>=216 && py<=258 && px>=112 && px<=248) return RoundHit::Details;
  if (px>=78 && px<=282 && py>=104 && py<=215) return RoundHit::Center;
  return RoundHit::None;
}
} // namespace pocket
