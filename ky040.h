/* 'high-level' controller for the ky-040 rotary encoder
 * Specifically aimed at the RaspberryPi
 * Requires the wiringPi library (RaspberryPi exclusive library?)
 * Copyright (C) ibmicroapple, 2017
 */

#ifndef KY040_H
#define KY040_H

#include <stdint.h>
#include <wiringPi.h>

typedef enum
{
	KY040_CLICK,
	KY040_RELEASE,
	KY040_LEFT,
	KY040_RIGHT
} KY040_EVENT;

#ifndef false
	#define false 0
#endif

#ifndef true
	#define true 1
#endif

typedef uint8_t ky040_bool;
typedef uint8_t ky040_register;
typedef uint8_t ky040_gpio;

typedef struct
{
	ky040_register	dt,
			clk,
			sw;
	ky040_gpio	pin_dt,
			pin_clk,
			pin_sw;
	ky040_bool	clicked;
	void		(*callback)(KY040_EVENT event_type);
} ky040;

#define KY040_NULL (ky040){0}

static ky040_bool ky040_initialized = false;

void ky040_init(ky040* const dev, const ky040_gpio pin_dt, const ky040_gpio pin_clk, const ky040_gpio pin_sw);
#define ky040_on_event(dev, fptr) ((ky040*)dev)->callback = fptr;
#define ky040_is_clicked(dev) ((ky040*)dev)->clicked
void ky040_update(ky040* const dev);

void ky040_init(ky040* const dev, const ky040_gpio pin_dt, const ky040_gpio pin_clk, const ky040_gpio pin_sw)
{
	if(!ky040_initialized)
	{
		if(wiringPiSetup() < 0) return;
		ky040_initialized = true;
	}

	*dev = KY040_NULL;

	pinMode(pin_dt, INPUT);
	pinMode(pin_clk, INPUT);
	pinMode(pin_sw, INPUT);

	pullUpDnControl(pin_sw, PUD_UP);

	dev->pin_dt = pin_dt;
	dev->pin_clk = pin_clk;
	dev->pin_sw = pin_sw;
}

void ky040_update(ky040* const dev)
{
	ky040_register new_clk;

	dev->dt = digitalRead(dev->pin_dt);
	dev->clk = digitalRead(dev->pin_clk);
	dev->sw = digitalRead(dev->pin_sw);

	while(!(dev->dt = digitalRead(dev->pin_dt)))
	{
		new_clk = digitalRead(dev->pin_clk);

		if(new_clk && !dev->clk)
		{
			if(dev->callback) dev->callback(KY040_LEFT);
		}
		else if(!new_clk && dev->clk)
		{
			if(dev->callback) dev->callback(KY040_RIGHT);
		}
	}

	if(!dev->sw)
	{
		if(!dev->clicked)
		{
			dev->clicked = true;
			if(dev->callback) dev->callback(KY040_CLICK);
		}
	}
	else
	{
		if(dev->clicked)
		{
			dev->clicked = false;
			if(dev->callback) dev->callback(KY040_RELEASE);
		}
	}
}

#endif /* KY040_H */
