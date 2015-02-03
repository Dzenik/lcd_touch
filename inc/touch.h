#ifndef TOUCH_H
#define TOUCH_H


void vTouchTask( void *pvParameters ) ;
void Touch_Initializtion(void);
uint16_t Touch_GetPhyX(void);
uint16_t Touch_GetPhyY(void);
int16_t  Touch_MeasurementX(void);
int16_t  Touch_MeasurementY(void);
portBASE_TYPE touchIsInWindow(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif
