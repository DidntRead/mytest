#ifndef FOCAL_IALGORITHM_H
#define FOCAL_IALGORITHM_H

#include <stdint.h>
#include <stdbool.h>

#include "focal_image.h"

#define FOCAL_IALGORITHM_VERSION  (1)

#define FOCAL_ENROL_DONE 100
#define FOCAL_TEMPLATE_UPDATED 1

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (focal_exit_condition_t) (void* user);
typedef struct _FocalEnrolSession enrolment_session_t;
typedef struct _focal_template_t focal_template_t;

typedef struct {
	int32_t y; //-1 indicates undefined
	int32_t x; //-1 indicates undefined
} FocalPoint;

typedef struct {
	FocalPoint bottom_left;
	FocalPoint bottom_right;
	FocalPoint top_left;
	FocalPoint top_right;

} FocalRect;

typedef struct {
	FocalRect* masks;
	int nr_of_masks;
} FocalMaskList;

typedef struct focal_image_quality_t {
    int coverage;
    int quality;
    int acceptance;
    int reject_reason;
    int stitched;
} focal_image_quality_t;

typedef struct ialgorithm_t {
    void (*enrolStart)();
    int32_t (*enrolAddImage)(focal_image_quality_t* image_quality);
    int32_t (*enrolFinish)(void);
    int8_t (*enrolGetProgress)(int8_t* touches_very_immobile);
    int32_t (*enrolGetSuggestedDirectionOfNextTouch)();
    int8_t (*enrolMainClusterIdentified)();
    bool (*enrolGetLastTouch)(FocalRect* rect);
    bool (*enrolGetNextTouch)(FocalRect* rect);
    bool (*enrolGetStitchedMaskList)(FocalRect* rectList);
    int (*enrolGetStitchedMaskListSize)();
    int8_t (*enrolEstimateFingerSize)();
    int32_t (*debugRetrieveImage)(uint8_t* image, uint32_t* size);
    int32_t (*debugInjectImage)(uint8_t* image, uint32_t size);
    int32_t (*identifyStart)(uint32_t* candidates, uint32_t candidate_count);
    int32_t (*identifyImage)(int16_t* result,
                             int32_t* score,
                             focal_image_quality_t* image_quality);
    int32_t (*identifyFinish)(void);
    uint32_t (*removeTemplate)(uint32_t id);
    void (*getIds)(uint32_t* ids, uint32_t* len);
    uint32_t (*getIdsCount)(void);
    uint32_t (*waitForFingerPresent)(void);
    uint32_t (*waitForFingerLost)(void);
    uint32_t (*captureImage)(void);
    int (*getDBIndexByID)(uint32_t id);
    /*return 0 if deadpixel test passed, otherwise deadpixel test fails, the number of dead pixels can is returned in nDeadPixels*/
    int (*deadPixelTest)(int32_t* nDeadPixels);
    uint32_t (*removeAllTemplates)();
    void (*abort)();
    uint32_t (*wakeupSetup)(void);

} ialgorithm_t;

void getIalgorithm(ialgorithm_t* ialgorithm);
int32_t getIalgorithmVersion(void);

#ifdef __cplusplus
}
#endif
#endif // FOCAL_ALGORITHM_H
