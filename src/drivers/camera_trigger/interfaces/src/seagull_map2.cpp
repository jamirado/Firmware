#ifdef __PX4_NUTTX

#include <sys/ioctl.h>
#include <lib/mathlib/mathlib.h>

#include "drivers/drv_pwm_trigger.h"
#include "seagull_map2.h"

// PWM levels of the interface to Seagull MAP 2 converter to
// Multiport (http://www.seagulluav.com/manuals/Seagull_MAP2-Manual.pdf)
#define PWM_CAMERA_DISARMED			900
#define PWM_CAMERA_ON				1100
#define PWM_CAMERA_AUTOFOCUS_SHOOT	1300
#define PWM_CAMERA_NEUTRAL			1500
#define PWM_CAMERA_INSTANT_SHOOT	1700
#define PWM_CAMERA_OFF				1900
#define PWM_2_CAMERA_KEEP_ALIVE		1700
#define PWM_2_CAMERA_ON_OFF			1900

CameraInterfaceSeagull::CameraInterfaceSeagull():
	CameraInterface(),
	_camera_is_on(false)
{
	get_pins();
	setup();
}

CameraInterfaceSeagull::~CameraInterfaceSeagull()
{
	// Deinitialise trigger channels
	up_pwm_trigger_deinit();
}

void CameraInterfaceSeagull::setup()
{
	for (unsigned i = 0; i < arraySize(_pins); i = i + 2) {
		if (_pins[i] >= 0 && _pins[i + 1] >= 0) {
			uint8_t pin_bitmask = (1 << _pins[i + 1]) | (1 << _pins[i]);
			up_pwm_trigger_init(pin_bitmask);
			up_pwm_trigger_set(_pins[i + 1], math::constrain(PWM_CAMERA_DISARMED, PWM_CAMERA_DISARMED, 2000));
			up_pwm_trigger_set(_pins[i], math::constrain(PWM_CAMERA_DISARMED, PWM_CAMERA_DISARMED, 2000));

			// We only support 2 consecutive pins while using the Seagull MAP2
			return;
		}
	}

	PX4_ERROR("Bad pin configuration - Seagull MAP2 requires 2 consecutive pins for control.");
}

void CameraInterfaceSeagull::trigger(bool enable)
{

	if (!_camera_is_on) {
		return;
	}

	for (unsigned i = 0; i < arraySize(_pins); i = i + 2) {
		if (_pins[i] >= 0 && _pins[i + 1] >= 0) {
			// Set all valid pins to shoot or neutral levels
			up_pwm_trigger_set(_pins[i + 1], math::constrain(enable ? PWM_CAMERA_INSTANT_SHOOT : PWM_CAMERA_NEUTRAL, 1000, 2000));
		}
	}
}

void CameraInterfaceSeagull::keep_alive(bool signal_on)
{
	// This should alternate between signal_on and !signal_on to keep the camera alive

	if (!_camera_is_on) {
		return;
	}

	for (unsigned i = 0; i < arraySize(_pins); i = i + 2) {
		if (_pins[i] >= 0 && _pins[i + 1] >= 0) {
			// Set channel 2 pin to keep_alive or netural signal
			up_pwm_trigger_set(_pins[i], math::constrain(signal_on ? PWM_2_CAMERA_KEEP_ALIVE : PWM_CAMERA_NEUTRAL, 1000, 2000));
		}
	}
}

void CameraInterfaceSeagull::turn_on_off(bool enable)
{

	for (unsigned i = 0; i < arraySize(_pins); i = i + 2) {
		if (_pins[i] >= 0 && _pins[i + 1] >= 0) {
			// For now, set channel one to neutral upon startup.
			up_pwm_trigger_set(_pins[i + 1], math::constrain(PWM_CAMERA_NEUTRAL, 1000, 2000));
			up_pwm_trigger_set(_pins[i], math::constrain(enable ? PWM_2_CAMERA_ON_OFF : PWM_CAMERA_NEUTRAL, 1000, 2000));
		}
	}

	if (!enable) { _camera_is_on = !_camera_is_on; }
}

void CameraInterfaceSeagull::info()
{
	PX4_INFO("PWM trigger mode (Seagull MAP2) , pins enabled : [%d][%d][%d][%d][%d][%d]",
		 _pins[5], _pins[4], _pins[3], _pins[2], _pins[1], _pins[0]);
}

#endif /* ifdef __PX4_NUTTX */
