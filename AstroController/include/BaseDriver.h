#ifndef INDIDRIVER_H_
#define INDIDRIVER_H_

#include "Scheduled.h"
#include "IndiNumberVector.h"
#include "IndiIntVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"


#define TELESCOPE_INTERFACE     (0)  /**< Telescope interface, must subclass INDI::Telescope */
#define CCD_INTERFACE           (1)  /**< CCD interface, must subclass INDI::CCD */
#define GUIDER_INTERFACE        (2)  /**< Guider interface, must subclass INDI::GuiderInterface */
#define FOCUSER_INTERFACE       (3)  /**< Focuser interface, must subclass INDI::FocuserInterface */
#define FILTER_INTERFACE        (4)  /**< Filter interface, must subclass INDI::FilterInterface */
#define DOME_INTERFACE          (5)  /**< Dome interface, must subclass INDI::Dome */
#define GPS_INTERFACE           (6)  /**< GPS interface, must subclass INDI::GPS */
#define WEATHER_INTERFACE       (7)  /**< Weather interface, must subclass INDI::Weather */
#define AO_INTERFACE            (8)  /**< Adaptive Optics Interface */
#define DUSTCAP_INTERFACE       (9)  /**< Dust Cap Interface */
#define LIGHTBOX_INTERFACE      (10) /**< Light Box Interface */
#define DETECTOR_INTERFACE      (11) /**< Detector interface, must subclass INDI::Detector */
#define ROTATOR_INTERFACE       (12) /**< Rotator interface, must subclass INDI::RotatorInterface */
#define SPECTROGRAPH_INTERFACE  (13) /**< Spectrograph interface */
#define AUX_INTERFACE           (15) /**< Auxiliary interface */


class BaseDriver {
    uint16_t ival;
    Symbol group;
    IndiTextVector driverInfo;
    IndiTextVectorMember interface;

public:
    BaseDriver();

    void addCapability(uint8_t capId);
};

#endif
