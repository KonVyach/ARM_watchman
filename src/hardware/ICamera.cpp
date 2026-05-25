#include "facecontrol/hardware/ICamera.h"
namespace fc {
// Out-of-line destructor anchors the vtable and provides a translation unit
// for AUTOMOC to generate the Q_OBJECT meta-object machinery.
ICamera::~ICamera() = default;
}
