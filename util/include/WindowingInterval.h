#ifndef WINDOWINGINTERVAL_H
#define WINDOWINGINTERVAL_H



//W-350 C -24
//W-115 C -220
//Centro W-255 C-128


//Stl includes
#include <iostream>

class WindowingInterval{

    public:
        static const uint8_t BRAIN = 1;
        static const uint8_t SUBDURAL = 2;
        static const uint8_t STROKE = 3;
        static const uint8_t BRAIN_TEMP_BONES = 4;
        static const uint8_t BRAIN_SOFT_TISSUES = 5;
        static const uint8_t LUNG = 6;
        static const uint8_t MEDIASTINUM = 7;
        static const uint8_t ABDOMEN_SOFT_TISSUES = 8;
        static const uint8_t LIVER = 9;
        static const uint8_t SPINE_SOFT_TISSUES = 10;
        static const uint8_t SPINE_BONE = 11;
        static const uint8_t MAMMOGRAM_8 = 12;

    public:
        WindowingInterval();

        static uint16_t getCenter(uint8_t const &reference);
        static uint16_t getWidth(uint8_t const &reference);
        static uint16_t getMargin(uint8_t const &reference);
        static uint16_t getStep(uint8_t const &reference);
};


/** Baseline info ***/
/** CT
head and neck
brain W:80 L:40
subdural W:130-300 L:50-100
stroke W:8 L:32 or W:40 L:40 3
temporal bones W:2800 L:600
soft tissues: W:350–400 L:20–60 4
chest
lungs W:1500 L:-600
mediastinum W:350 L:50
abdomen
soft tissues W:400 L:50
liver W:150 L:30
spine
soft tissues W:250 L:50
bone W:1800 L:400
**/

#endif // WINDOWINGINTERVAL_H
