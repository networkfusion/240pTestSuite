/* 
 * 240p Test Suite for Nintendo 64
 * Copyright (C)2024 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 */
 
#include "menu.h"

typedef struct timecode {
	int hours;
	int minutes;
	int seconds;
	int frames;
	int type;
	int res;
} timecode;

uint16_t convertToFrames(timecode *time) {
	uint16_t	frames = 0;

	if(!time)
		return frames;

	frames = time->frames;
	if(isPAL)
		frames += time->seconds*50;
	else
		frames += time->seconds*60;
	frames += time->minutes*3600;
	frames += time->hours*216000;
	return frames;
}

void convertFromFrames(timecode *value, uint16_t Frames) {
	if(!value)
		return;
	value->hours = Frames / 216000;
	Frames = Frames % 216000;
	value->minutes = Frames / 3600;
	Frames = Frames % 3600;
	if(isPAL) {
		value->seconds = Frames / 50;
		value->frames = Frames % 50;
	}
	else {
		value->seconds = Frames / 60;
		value->frames = Frames % 60;
	}
}

void drawSonicBG(int x, int y, image *sonicTop, image *sonicWater, image *sonicFall) {	
	rdpqClearScreen();
	/* Draw Background */
	if(x > 0) {
		rdpqDrawImageXY(sonicTop, x-256, 0);
		rdpqDrawImageXY(sonicWater, x-256, 156);
		rdpqDrawImageXY(sonicFall, x-177, 131);
	}
	rdpqDrawImageXY(sonicTop, x, 0);
	rdpqDrawImageXY(sonicWater, x, 156);
	rdpqDrawImageXY(sonicFall, x+79, 131);
	if(x < 64) {
		rdpqDrawImageXY(sonicTop, x+256, 0);
		rdpqDrawImageXY(sonicWater, x+256, 156);
		rdpqDrawImageXY(sonicFall, x+335, 131);
	}	
}

void drawSonicFG(int x, int y, image *overlay) {	
	/* Draw Foreground */
	if(x > 0)
		rdpqDrawImageXY(overlay, 2*x-256, 48);
	rdpqDrawImageXY(overlay, 2*x, 48);
	if(x < 64)
		rdpqDrawImageXY(overlay, 2*x+256, 48);
	// Extra gap
	if(x < -96)
		rdpqDrawImageXY(overlay, 2*x+512, 48);
}

void rotateWaterPalette(image *data) {
	uint16_t tmpCol = data->palette[2];
	
	data->palette[2] = data->palette[3];
	data->palette[3] = tmpCol;
	
	updatePalette(data);
}

void rotateFallPalette(image *data) {
	uint16_t tmpCol = data->palette[4];
	
	data->palette[4] = data->palette[3];
	data->palette[3] = data->palette[2];
	data->palette[2] = tmpCol;
	
	updatePalette(data);
}

void drawDropShadow(int striped) {
	resolution_t oldVmode = current_resolution;
	int end = 0, invert = 0, frame = 0, reload = 1;
	int x = 0, y = 0, showText = 0, sprite = 0, selback = 0;
	int frameLen[] = { 8, 8, 8 }, animFrame = 0;
	int fallFrame = 0, currentframe = 0;
	image *donna = NULL, *shadow = NULL, *stripes = NULL, *check = NULL;
	image *buzz = NULL, *buzzshadow = NULL, *sshadow = NULL;
	image *sonicTop = NULL, *sonicWater = NULL, *sonicFall = NULL;
	image *overlay = NULL;	
	joypad_buttons_t keys;
	char msg[25];

	if(!striped) {
		// Regular test
		sshadow = loadImage("rom:/shadow.sprite");
		if(sshadow)	{
			x = dW/2- sshadow->tiles->width/2;
			y = dH/2 - sshadow->tiles->height/2;
		}
		
		buzz = loadImage("rom:/buzzbomber.sprite");
		buzzshadow = loadImage("rom:/buzzbomberShadow.sprite");
	
		sprite = RANDN(2);
		if(sprite == 0)
			shadow = sshadow;
		else
			shadow = buzzshadow;
	}
	else {
		shadow = loadImage("rom:/striped.sprite");
		if(shadow)	{
			x = dW/2- shadow->tiles->width/2;
			y = dH/2 - shadow->tiles->height/2;
		}
	}
		
	sonicTop = loadImage("rom:/sonicTop.sprite");
	sonicWater = loadImage("rom:/sonicWater.sprite");
	sonicFall = loadImage("rom:/sonicFall.sprite");
	overlay = loadImage("rom:/sonicFloor.sprite");
	
	check = loadImage("rom:/check.sprite");
	stripes = loadImage("rom:/stripes_v.sprite");

	while(!end) {
		if(reload) {
			if(!isSameRes(&oldVmode, &current_resolution)) {
				freeImage(&donna);
				oldVmode = current_resolution;
			}
			
			if(!donna) {
				if(vMode == SUITE_640x480) {
					donna = loadImage("rom:/donna-480.sprite");
					if(!donna)
						end = 1;
					else
						donna->scale = 0;
				}
				else
					donna = loadImage("rom:/donna.sprite");
				if(!donna)
					end = 1;
			}

			reload = 0;
		}
		
		
		getDisplay();

		rdpqStart();
		
		switch(selback) {
			case 0:
				rdpqDrawImage(donna);
				break;
			case 1:
				drawSonicBG(-(x-196), y, sonicTop, sonicWater, sonicFall);
				break;
			case 2:
				rdpqFillWithImage(stripes);
				break;
			case 3:
				rdpqFillWithImage(check);
				break;
			default:
				break;
		}
		
		if(!striped) {
			if(frame == invert)
				rdpqDrawImageXY(shadow, x, y);
			if(sprite == 1 && buzz)	{
				rdpqDrawImageXY(buzz, x-20, y-20);
			}
		}
		else
			rdpqDrawImageXY(shadow, x, y);
		
		if(selback == 1)
			drawSonicFG(-(x-128), y, overlay);
		rdpqEnd();
		
		if(!striped && showText) {
			if(vMode == SUITE_640x480)
				drawStringB(450, 40, 0, 0xff, 0, msg);
			else
				drawStringB(140, 30, 0, 0xff, 0, msg);
			showText--;
		}
		
		checkMenu(striped ? STRIPED : DROPSHADOW, &reload);
		waitVsync();
		
		frame = !frame;
		
		joypad_poll();
		keys = controllerButtonsHeld();
		
		if(keys.d_up)
			y--;
		if(keys.d_down)
			y++;
		if(y < 0)
			y = 0;
		if(y > dH - shadow->tiles->height)
			y = dH - shadow->tiles->height;
			
		if(keys.d_left) {
			x--;
			if(!striped) {
				buzz->flipH = 0;
				buzzshadow->flipH = 0;
			}
		}
		if(keys.d_right) {
			x++;
			if(!striped) {
				buzz->flipH = 1;
				buzzshadow->flipH = 1;
			}
		}
		if(x < 0)
			x = 0;
		if(x > dW - shadow->tiles->width)
			x = dW - shadow->tiles->width;
			
		if(selback == 1) {
			currentframe ++;
			if(currentframe > 25) {
				rotateWaterPalette(sonicWater);
				currentframe = 0;
			}
			fallFrame ++;
			if(fallFrame > frameLen[animFrame]) {
				rotateFallPalette(sonicFall);
				fallFrame = 0;
				animFrame ++;
				if(animFrame > 2)
					animFrame = 0;
			}
		}

		keys = controllerButtonsDown();
		checkStart(keys);
		if(keys.b)
			end = 1;
		
		if(!striped && keys.r) {
			invert = !invert;
			if(invert)
				sprintf(msg, "Shadow on odd frames");				
			else
				sprintf(msg, "Shadow on even frames");
			showText = 60;
		}
		
		if(!striped && keys.l) {
			sprite = !sprite;
			if(sprite == 0)
				shadow = sshadow;
			else
				shadow = buzzshadow;
		}
		
		if(keys.a) {
			selback ++;
			if(selback > 3)
				selback = 0;
		}
	}
	
	if(!striped) {
		freeImage(&sshadow);
		freeImage(&buzzshadow);
		freeImage(&buzz);
	}
	else
		freeImage(&shadow);
	
	freeImage(&donna);
	
	freeImage(&sonicTop);
	freeImage(&sonicWater);
	freeImage(&sonicFall);
	freeImage(&overlay);
	
	freeImage(&stripes);
	freeImage(&check);
}

void drawLagTest() {
	uint16_t	frames = 0, seconds = 0, minutes = 0, hours = 0, framecnt = 1, done =  0;
	uint16_t	lsd, msd, pause = 0, toggle = 0;		
	image		*blueCircle = NULL, *redCircle = NULL, *circle = NULL;
	joypad_buttons_t keys;

	blueCircle = loadImage("rom:/circle.sprite");
	if(!blueCircle)
		return;
	
	redCircle = loadImage("rom:/circle.sprite");
	if(!redCircle)
		return;
	
	loadNumbers();
	
	blueCircle->palette[2] = graphics_make_color(0x00, 0x00, IRE_100, 0xff);
	updatePalette(blueCircle);
	redCircle->palette[2] = graphics_make_color(IRE_100, 0x00, 0x00, 0xff);
	updatePalette(redCircle);

	while(!done) {		
		if(!pause) {
			frames ++;
			framecnt ++;
			if(framecnt > 8)
				framecnt = 1;
			toggle = !toggle;
		}
		
		if(isPAL) {
			if(frames > 49)	{
				frames = 0;
				seconds ++;
			}
		}
		else {
			if(frames > 59) {
				frames = 0;
				seconds ++;
			}
		}

		if(seconds > 59) {
			seconds = 0;
			minutes ++;
		}

		if(minutes > 59) {
			minutes = 0;
			hours ++;
		}

		if(hours > 99)
			hours = 0;

		getDisplay();

		rdpqStart();
		rdpqClearScreenWhite();

		// Counter Separators
		drawDigit(80, 16, NUMBER_BLACK, 10);
		drawDigit(152, 16, NUMBER_BLACK, 10);
		drawDigit(224, 16, NUMBER_BLACK, 10);

		// Circles 1st row
		if(framecnt == 1)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 8, 56);
		drawDigit(28, 68, NUMBER_WHITE, 1);

		if(framecnt == 2)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 88, 56);
		drawDigit(108, 68, NUMBER_WHITE, 2);

		if(framecnt == 3)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 168, 56);
		drawDigit(188, 68, NUMBER_WHITE, 3);

		if(framecnt == 4)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 248, 56);
		drawDigit(268, 68, NUMBER_WHITE, 4);

		// Circles 2nd row
		if(framecnt == 5)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 8, 136);
		drawDigit(28, 148, NUMBER_WHITE, 5);

		if(framecnt == 6)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 88, 136);
		drawDigit(108, 148, NUMBER_WHITE, 6);

		if(framecnt == 7)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 168, 136);
		drawDigit(188, 148, NUMBER_WHITE, 7);

		if(framecnt == 8)
			circle = redCircle;
		else
			circle = blueCircle;
			
		rdpqDrawImageXY(circle, 248, 136);
		drawDigit(268, 148, NUMBER_WHITE, 8);

		// Draw Hours
		lsd = hours % 10;
		msd = hours / 10;
		drawDigit(32, 16, NUMBER_BLACK, msd);
		drawDigit(56, 16, NUMBER_BLACK, lsd);

		// Draw Minutes
		lsd = minutes % 10;
		msd = minutes / 10;
		drawDigit(104, 16, NUMBER_BLACK, msd);
		drawDigit(128, 16, NUMBER_BLACK, lsd);

		// Draw Seconds
		lsd = seconds % 10;
		msd = seconds / 10;
		drawDigit(176, 16, NUMBER_BLACK, msd);
		drawDigit(200, 16, NUMBER_BLACK, lsd);

		// Draw Frames
		lsd = frames % 10;
		msd = frames / 10;
		drawDigit(248, 16, toggle ? NUMBER_RED : NUMBER_BLUE, msd);
		drawDigit(272, 16, toggle? NUMBER_RED : NUMBER_BLUE, lsd);

		rdpqEnd();
		
		if(toggle) {
			drawBlackBox(0, 0, 6, dH);
			drawBlackBox(314, 0, 6, dH);
		}

		drawString(32, 8, 0, 0,	0, "hours");
		drawString(104, 8, 0, 0, 0, "minutes");
		drawString(176, 8, 0, 0, 0, "seconds");
		drawString(248, 8, 0, 0, 0, "frames");
		
		checkMenu(PASSIVELAG, NULL);
		waitVsync();

		joypad_poll();
		keys = controllerButtonsDown();
					
		if(keys.b)
			done =	1;				
					
		if(keys.c_left && !pause) {
			frames = hours = minutes = seconds = 0;
			framecnt = 1;
		}

		if(keys.a)
			pause = !pause;
		
		checkStart(keys);
	}
	
	freeImage(&redCircle);
	freeImage(&blueCircle);
	releaseNumbers();
}
 
void drawScroll() {
	int speed = 1, acc = -1, pause = 0, vertical = 0;
	int end = 0, x = 0, y = 0, currentframe = 0;
	int frameLen[] = { 8, 8, 8 }, animFrame = 0, fallFrame = 0;
	image 	*sonicTop = NULL, *sonicWater = NULL, *sonicFall = NULL;
	image	*overlay = NULL, *kiki = NULL;
	joypad_buttons_t keys;

	sonicTop = loadImage("rom:/sonicTop.sprite");
	sonicWater = loadImage("rom:/sonicWater.sprite");
	sonicFall = loadImage("rom:/sonicFall.sprite");
	overlay = loadImage("rom:/sonicFloor.sprite");
	kiki = loadImage("rom:/kiki.sprite");

	while(!end) {
		getDisplay();

		rdpqStart();
		if(!vertical) {
			drawSonicBG(x, y, sonicTop, sonicWater, sonicFall);
			drawSonicFG(x, y, overlay);
		}
		else {
			if(y > 0)
				rdpqDrawImageXY(kiki, 0, y-512);
			rdpqDrawImageXY(kiki, 0, y);
			if(y <= -272)
				rdpqDrawImageXY(kiki, 0, 512+y);
		}
		rdpqEnd();
		
		checkMenu(SCROLLHELP, NULL);
		waitVsync();
		
		joypad_poll();
		keys = controllerButtonsDown();
			
		if(keys.d_up)
			speed += 1;

		if(keys.d_down)
			speed -= 1;
		
		if(keys.b)
			end = 1;
		
		if(keys.a)
			pause = !pause;
		
		if(keys.c_left)
			acc *= -1;

		if(keys.c_right) {
			vertical = !vertical;
			
			if(vertical)
				changeToH256onVBlank();
			else
				changeToH320onVBlank();
		}
		
		if(speed > 16)
			speed = 16;

		if(speed < 1)
			speed = 1;

		checkStart(keys);

		if(!pause) {
			if(!vertical)
				x += speed * acc;
			else
				y -= speed * acc;
		}
		
		if(x < -128)
			x = 128 - speed;
		if(x > 128)
			x = -128 + speed;
		
		if(y >= 512)
			y = 0 + speed;
		if(y <= -512)
			y = 0 - speed;
		
		if(!vertical) {
			currentframe ++;
			if(currentframe > 25) {
				rotateWaterPalette(sonicWater);
				currentframe = 0;
			}
			fallFrame ++;
			if(fallFrame > frameLen[animFrame]) {
				rotateFallPalette(sonicFall);
				fallFrame = 0;
				animFrame ++;
				if(animFrame > 2)
					animFrame = 0;
			}
		}
	}
	
	freeImage(&sonicTop);
	freeImage(&sonicWater);
	freeImage(&sonicFall);
	freeImage(&overlay);
	freeImage(&kiki);
	
	if(isVMode256())
		changeToH320onVBlank();
}

void drawStripes() {
	int end = 0, alternate = 0, isVertical = 0;
	int frame = 0, dframe = 0;
	image *back = NULL, *horizontal = NULL, *vertical = NULL;;
	joypad_buttons_t keys;
	fmenuData	verMenuData[] = { { 0, "Horizontal" }, {1, "Vertical"} };
	
	horizontal = loadImage("rom:/stripes.sprite");
	if(!horizontal)
		return;
	
	vertical = loadImage("rom:/stripes_v.sprite");
	if(!vertical)
		return;
	
	isVertical = selectMenu("Select Type", verMenuData, 2, isVertical+1);
	if(isVertical == MENU_CANCEL)
		end = 1;
	
	while(!end) {
		getDisplay();

		back = isVertical ? vertical : horizontal;
		rdpqStart();
		rdpqFillWithImage(back);
		rdpqEnd();
		
		if(dframe) {
			char msg[20];

			sprintf(msg, "Frame: %02d", frame);
			drawStringB(20, vMode == SUITE_640x480 ? 460 : 210, 0xff, 0xff, 0xff, msg);
			frame ++;
			if(isPAL) {
				if(frame > 49)
					frame = 0;
			}
			else {
				if(frame > 59)
					frame = 0;
			}
		}
		checkMenu(STRIPESHELP, NULL);
		waitVsync();
		
		if(alternate)
			swapPaletteColors(back, 0, 1);
		
		joypad_poll();
		keys = controllerButtonsDown();
		
		checkStart(keys);
		if(keys.b)
			end = 1;
		if(keys.a)
			alternate = !alternate;
		
		if(keys.c_left && !alternate)
			swapPaletteColors(back, 0, 1);
		
		if(keys.c_right) {
			dframe = !dframe;
			frame = 0;
		}
		
		if(keys.r) {
			int oldOption = isVertical;
			
			isVertical = selectMenu("Select Type", verMenuData, 2, isVertical+1);
			if(isVertical == MENU_CANCEL)
				isVertical = oldOption;
		}
	}
	
	back = NULL;
	freeImage(&horizontal);
	freeImage(&vertical);
}

void drawCheckerBoard() {
	int end = 0, alternate = 0;
	int frame = 0, dframe = 0;
	image *back = NULL;
	joypad_buttons_t keys;
	
	back = loadImage("rom:/check.sprite");
	if(!back)
		return;
	while(!end) {
		getDisplay();

		rdpqStart();
		rdpqFillWithImage(back);
		rdpqEnd();
		
		if(dframe) {
			char msg[20];

			sprintf(msg, "Frame: %02d", frame);
			drawStringB(20, vMode == SUITE_640x480 ? 460 : 210, 0xff, 0xff, 0xff, msg);
			frame ++;
			if(isPAL) {
				if(frame > 49)
					frame = 0;
			}
			else {
				if(frame > 59)
					frame = 0;
			}
		}
		checkMenu(CHECKHELP, NULL);
		waitVsync();
		
		if(alternate)
			swapPaletteColors(back, 0, 1);
		
		joypad_poll();
		keys = controllerButtonsDown();
		
		checkStart(keys);
		if(keys.b)
			end = 1;
		if(keys.a)
			alternate = !alternate;
		
		if(keys.c_left && !alternate)
			swapPaletteColors(back, 0, 1);
		
		if(keys.c_right) {
			dframe = !dframe;
			frame = 0;
		}
	}
	
	freeImage(&back);
}

void SD_blink_cycle_phase(image *sd_b1, image *sd_b2, int index) {
	static int blink_counter[5] = { 0, 0, 0, 0, 0 };
	static int is_blinking[5] = { 0, 0, 0, 0, 0 };
	
	blink_counter[index]++;	
	if(sd_b1 && sd_b2 && blink_counter[index] > 230) {
		if(!is_blinking[index])	{
			if(RANDN(100) < 25) {
				is_blinking[index] = 1;
				blink_counter[index] = 230;
				rdpqDrawImage(sd_b1);
			}
		}
		else {
			if(blink_counter[index] >= 232 && blink_counter[index] < 234)
				rdpqDrawImage(sd_b2);
				
			if(blink_counter[index] >= 234 && blink_counter[index] < 238)
				rdpqDrawImage(sd_b1);
	
			if(blink_counter[index] >= 238)	{	
				blink_counter[index] = 0;
				is_blinking[index] = 0;
			}
		}
	}
}

void drawPhase() {
	int 		done = 0, type = 0, hpos = 0, i = 0;
	image		*back = NULL, *backcheck = NULL;
	image		*sd[5] = { NULL, NULL, NULL, NULL, NULL};
	image		*sd_b1[5] = { NULL, NULL, NULL, NULL, NULL};
	image		*sd_b2[5] = { NULL, NULL, NULL, NULL, NULL};
	joypad_buttons_t keys;
	
	back = loadImage("rom:/phase.sprite");
	if(!back)
		return;
	
	backcheck = loadImage("rom:/check.sprite");
	if(!backcheck)
		return;

	for(i = 0; i < 5; i++) {
		sd[i] = loadImage("rom:/sd.sprite");
		if(!sd[i])
			return;
		sd[i]->x = 5+i*64;
		sd[i]->y = 70;
		sd_b1[i] = loadImage("rom:/sd_b1.sprite");
		if(sd_b1[i]) {
			sd_b1[i]->x = sd[i]->x+16;
			sd_b1[i]->y = sd[i]->y+32;
		}
		
		sd_b2[i] = loadImage("rom:/sd_b2.sprite");
		if(sd_b2[i]) {
			sd_b2[i]->x = sd[i]->x+16;
			sd_b2[i]->y = sd[i]->y+32;
		}
	}

	while(!done) {
		getDisplay();
		
		rdpqStart();
		if(!type)
			rdpqDrawImage(back);
		else
			rdpqFillWithImage(backcheck);
		for(i = 0; i < 5; i++)
		{
			sd[i]->x = 5+i*64+hpos;
			rdpqDrawImage(sd[i]);
			if(sd_b1[i] && sd_b2[i]) {
				sd_b1[i]->x = sd[i]->x+16;
				sd_b2[i]->x = sd[i]->x+16;
				SD_blink_cycle_phase(sd_b1[i], sd_b2[i], i);
			}
		}
		rdpqEnd();
		
		checkMenu(PHASEHELP, NULL);
		waitVsync();
		
		joypad_poll();
		keys = controllerButtonsDown();

		checkStart(keys);
		if(keys.c_left)
			type = !type;
		
		if(keys.a)
			hpos = 0;
		
		if(keys.d_left)	{
			hpos --;
			if(hpos < -5)
				hpos = -5;
		}
		
		if(keys.d_right)	{
			hpos ++;
			if(hpos > 5)
				hpos = 5;
		}
		
		if(keys.b)
			done =	1;
	}
	
	freeImage(&back);
	freeImage(&backcheck);
	for(i = 0; i < 5; i++) {
		freeImage(&sd[i]);
		freeImage(&sd_b1[i]);
		freeImage(&sd_b2[i]);
	}
	return;
}

void drawDisappear() {
	int 		frames = 0, seconds = 0, minutes = 0, hours = 0;
	int			toggle = 0, done = 0;
	uint16_t	lsd, msd, show = 1;		
	image		*sd = NULL;
	int			x = -8, y = 10;
	joypad_buttons_t keys;
	
	sd = loadImage("rom:/sd.sprite");
	if(sd) {
		sd->x = 128;
		sd->y = 85;
	}
			
	loadNumbers();
	while(!done) {
		getDisplay();

		rdpqStart();
		if(toggle == 0)
			rdpqClearScreen();
		else
			rdpqClearScreenWhite();

		// Counter Separators
		drawDigit(x+80,  y+16, NUMBER_WHITE, 10);
		drawDigit(x+152, y+16, NUMBER_WHITE, 10);
		drawDigit(x+224, y+16, NUMBER_WHITE, 10);

		// Draw Hours
		lsd = hours % 10;
		msd = hours / 10;
		drawDigit(x+32, y+16, NUMBER_WHITE, msd);
		drawDigit(x+56, y+16, NUMBER_WHITE, lsd);

		// Draw Minutes
		lsd = minutes % 10;
		msd = minutes / 10;
		drawDigit(x+104, y+16, NUMBER_WHITE, msd);
		drawDigit(x+128, y+16, NUMBER_WHITE, lsd);

		// Draw Seconds
		lsd = seconds % 10;
		msd = seconds / 10;
		drawDigit(x+176, y+16, NUMBER_WHITE, msd);
		drawDigit(x+200, y+16, NUMBER_WHITE, lsd);

		// Draw Frames
		lsd = frames % 10;
		msd = frames / 10;
		drawDigit(x+248, y+16, NUMBER_WHITE, msd);
		drawDigit(x+272, y+16, NUMBER_WHITE, lsd);
		
		if(show) {
			rdpqDrawImage(sd);
			SD_blink_cycle(sd);
		}
		
		rdpqEnd();
		
		drawString(x+32,  y+8, 0xff, 0xff, 0xff, "hours");
		drawString(x+104, y+8, 0xff, 0xff, 0xff, "minutes");
		drawString(x+176, y+8, 0xff, 0xff, 0xff, "seconds");
		drawString(x+248, y+8, 0xff, 0xff, 0xff, "frames");

		checkMenu(DISAPPEAR, NULL);
		waitVsync();
		
		if(toggle)
			toggle --;
	
		joypad_poll();
		keys = controllerButtonsDown();
		if(keys.b)
			done =	1;
		
		if(keys.a)
			show = !show;
			
		if(keys.c_left)
			toggle = 2;
		
		checkStart(keys);

		frames ++;

		if(frames > (isPAL ? 49 : 59)) {
			frames = 0;
			seconds ++;
		}

		if(seconds > 59) {
			seconds = 0;
			minutes ++;
		}

		if(minutes > 59) {
			minutes = 0;
			hours ++;
		}

		if(hours > 99)
			hours = 0;
	}
	
	freeImage(&sd);
	releaseNumbers();
	
	SD_release();
}

void drawAlternate240p480i()
{
	int 			frames = 0, seconds = 0, minutes = 0, hours = 0;
	int				done =  0, current = 0, res = 0, status = 0;
	timecode		times[20];
	resolution_t	oldvmode = current_resolution;
	char 			buffer[100];
	joypad_buttons_t keys;
	
	if(isPAL) {
		if(videoModeToInt(&current_resolution) != SUITE_320x240) {
			setVideo(RESOLUTION_320x240);	
		}
	}
	else {
		if(videoModeToInt(&current_resolution) != SUITE_320x240) {		
			setVideo(RESOLUTION_320x240);
		}
	}
	
	setMenuVideo(0);
	useReducedWidthSpace(0);
	while(!done) {
		int x = 12;
		
		frames ++;

		if(isPAL) {
			if(frames > 49)	{
				frames = 0;
				seconds ++;
			}
		}
		else {
			if(frames > 59)	{
				frames = 0;
				seconds ++;
			}
		}

		if(seconds > 59) {
			seconds = 0;
			minutes ++;
		}

		if(minutes > 59) {
			minutes = 0;
			hours ++;
		}

		if(hours > 99)
			hours = 0;

		getDisplay();

		setClearScreen();
		rdpqStart();
		rdpqEnd();
		
		sprintf(buffer, "Current Resolution: %s", res == 0 ? (isPAL ? "288p" : "240p") : (isPAL ? "576i" : "480i"));
		drawString(x, 8, 0, 0xff, 0, buffer);

		sprintf(buffer, "Elapsed Timer:%02d:%02d:%02d:%02d", hours, minutes, seconds, frames);
		drawString(x, 32, 0xff, 0xff, 0xff, buffer);

		if(current)	{
			int i = 0;
			for(i = 0; i < current; i++) {
				if(times[i].type == 0) {
					sprintf(buffer, "-> to %s at: ",
						times[i].res == 0 ? (isPAL ? "288p" : "240p") : (isPAL ? "576i" : "480i"));
						
					drawString(x, 40+i*fh, 0xff, 0xff, 0x00, buffer);
				}
				else {
					sprintf(buffer, "Viewed at: ");
					drawString(x, 40+i*fh, 0xff, 0x00, 0x00, buffer);
				}

				sprintf(buffer, "%02d:%02d:%02d:%02d",
					times[i].hours, times[i].minutes, times[i].seconds, times[i].frames);
				drawString(x+14*fw, 40+i*fh, 0xff, 0xff, 0xff, buffer);
				
				if(times[i].type != 0 && i >= 1 && i <= 17)	{
					uint16_t	framesA = 0, framesB = 0, res = 0;
					timecode 	len;

					framesB = convertToFrames(&times[i]);
					framesA = convertToFrames(&times[i - 1]);
					res = framesB - framesA;
					convertFromFrames(&len, res);
					sprintf(buffer, "%02d:%02d:%02d:%02d", len.hours, len.minutes, len.seconds, len.frames);
					drawString(x+26*fw, 40+i*fh, 0xff, 0.0, 0.0, buffer);
				}
			}
		}
		
		checkMenu(ALTERNATE, NULL);
		waitVsync();

		joypad_poll();
		keys = controllerButtonsDown();
					
		checkStart(keys);

		if(keys.b)
			done =	1;				
					
		if(keys.a) {
			if(current <= 19)
				current ++;
			else
				current = 1;

			times[current - 1].frames = frames;
			times[current - 1].minutes = minutes;
			times[current - 1].seconds = seconds;
			times[current - 1].hours = hours;

			status ++;
			if(status == 1)	{
				times[current - 1].type = 0;
				res = !res;
				times[current - 1].res = res;	
				if(!res)
					setVideo(RESOLUTION_320x240);
				else
					setVideo(RESOLUTION_640x480);
			}
			if(status == 2)	{
				times[current - 1].type = 1;
				times[current - 1].res = res;
				status = 0;
			}
		}
	}	
	
	setMenuVideo(1);
	
	useReducedWidthSpace(1);
	setVideo(oldvmode);	
}
