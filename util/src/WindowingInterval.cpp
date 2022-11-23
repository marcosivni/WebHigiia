#include "WindowingInterval.h"

WindowingInterval::WindowingInterval(){
}

uint16_t WindowingInterval::getCenter(const uint8_t &reference){

    uint16_t answer;

    switch( reference )
    {
    case BRAIN:{
        answer = 40;
        break;
    }
    case SUBDURAL:{
        answer = 75;
        break;
    }
    case STROKE:{
        answer = 32;
        break;
    }
    case BRAIN_TEMP_BONES:{
        answer = 600;
        break;
    }
    case BRAIN_SOFT_TISSUES:{
        answer = 40;
        break;
    }
    case LUNG:{
        answer = -600;
        break;
    }
    case MEDIASTINUM:{
        answer = 50;
        break;
    }
    case ABDOMEN_SOFT_TISSUES:{
        answer = 50;
        break;
    }
    case LIVER:{
        answer = 30;
        break;
    }
    case SPINE_SOFT_TISSUES:{
        answer = 50;
        break;
    }
    case SPINE_BONE:{
        answer = 400;
        break;
    }
    case MAMMOGRAM_8:{
        answer = 32768;
        break;
    }
    default:
        answer = 400;
    }

    return answer;
}

uint16_t WindowingInterval::getWidth(const uint8_t &reference){

    uint16_t answer;

    switch( reference )
    {
    case BRAIN:{
        answer = 80;
        break;
    }
    case SUBDURAL:{
        answer = 215;
        break;
    }
    case STROKE:{
        answer = 8;
        break;
    }
    case BRAIN_TEMP_BONES:{
        answer = 2800;
        break;
    }
    case BRAIN_SOFT_TISSUES:{
        answer = 375;
        break;
    }
    case LUNG:{
        answer = 1500;
        break;
    }
    case MEDIASTINUM:{
        answer = 350;
        break;
    }
    case ABDOMEN_SOFT_TISSUES:{
        answer = 400;
        break;
    }
    case LIVER:{
        answer = 150;
        break;
    }
    case SPINE_SOFT_TISSUES:{
        answer = 250;
        break;
    }
    case SPINE_BONE:{
        answer = 1800;
        break;
    }
    case MAMMOGRAM_8:{
        answer = 65535;
        break;
    }
    default:
        answer = 400;
    }


    return answer;
}

uint16_t WindowingInterval::getMargin(const uint8_t &reference){

    uint16_t answer;

    switch( reference )
    {
    case BRAIN:{
        answer = 200;
        break;
    }
    case SUBDURAL:{
        answer = 300;
        break;
    }
    case STROKE:{
        answer = 50;
        break;
    }
    case BRAIN_TEMP_BONES:{
        answer = 1000;
        break;
    }
    case BRAIN_SOFT_TISSUES:{
        answer = 500;
        break;
    }
    case LUNG:{
        answer = 500;
        break;
    }
    case MEDIASTINUM:{
        answer = 200;
        break;
    }
    case ABDOMEN_SOFT_TISSUES:{
        answer = 300;
        break;
    }
    case LIVER:{
        answer = 200;
        break;
    }
    case SPINE_SOFT_TISSUES:{
        answer = 200;
        break;
    }
    case SPINE_BONE:{
        answer = 1500;
        break;
    }
    case MAMMOGRAM_8:{
        answer = 30000;
        break;
    }
    default:
        answer = 2000;
    }

    return answer;
}


uint16_t WindowingInterval::getStep(const uint8_t &reference){

    uint16_t answer;

    switch( reference )
    {
    case BRAIN:{
        answer = 5;
        break;
    }
    case SUBDURAL:{
        answer = 15;
        break;
    }
    case STROKE:{
        answer = 1;
        break;
    }
    case BRAIN_TEMP_BONES:{
        answer = 50;
        break;
    }
    case BRAIN_SOFT_TISSUES:{
        answer = 25;
        break;
    }
    case LUNG:{
        answer = 25;
        break;
    }
    case MEDIASTINUM:{
        answer = 5;
        break;
    }
    case ABDOMEN_SOFT_TISSUES:{
        answer = 10;
        break;
    }
    case LIVER:{
        answer = 5;
        break;
    }
    case SPINE_SOFT_TISSUES:{
        answer = 200;
        break;
    }
    case SPINE_BONE:{
        answer = 75;
        break;
    }
    case MAMMOGRAM_8:{
        answer = 100;
        break;
    }
    default:
        answer = 50;
    }

    return answer;
}


