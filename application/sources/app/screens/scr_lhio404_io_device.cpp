#include "scr_lhio404_io_device.h"

#define STEP_SETTING_CHOSSE (32) // increment the x asix as 32px, which is the width of each setting item on the screen.

#define SETTING_ITEM_ARRDESS_0 (0)
#define SETTING_ITEM_ARRDESS_1 (STEP_SETTING_CHOSSE)
#define SETTING_ITEM_ARRDESS_2 (STEP_SETTING_CHOSSE * 2)
#define SETTING_ITEM_ARRDESS_3 (STEP_SETTING_CHOSSE * 3)
#define SETTING_ITEM_ARRDESS_4 (STEP_SETTING_CHOSSE * 4)

static void view_scr_lhio404_io_device();

/* Move focus to the next/previous box based on the delta (1 for next, -1 for previous) */
static void scr_lhio404_io_device_move_focus(int8_t delta)
{
	const int8_t boxCount = 4;
	int8_t focusItem = scr_lhio404_io_device.focus_item + delta;

	if (focusItem < 0)
	{
		focusItem = boxCount - 1;
	}
	else if (focusItem >= boxCount)
	{
		focusItem = 0;
	}

	scr_lhio404_io_device.focus_item = focusItem;
	view_scr_lhio404_io_device();
}

view_dynamic_t dyn_view_item_lhio404_io_device = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_lhio404_io_device};

view_screen_t scr_lhio404_io_device = {
	&dyn_view_item_lhio404_io_device,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

/* This function renders the LHIO404 IO Device screen, showing the status of 4 relay outputs and allowing toggling them */
void view_scr_lhio404_io_device()
{
	char str[4];

	view_render.clear();
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.fillRoundRect(0, 0, 42, 14, 1, WHITE);

	view_render.setTextSize(1);
	view_render.setTextColor(BLACK);
	view_render.setCursor(8, 4);
	view_render.print("RELAY");

	for (uint8_t boxIndex = 0; boxIndex < 4; ++boxIndex)
	{
		const int16_t boxX = 4 + 32 * boxIndex;

		if (boxIndex == scr_lhio404_io_device.focus_item)
		{
			view_render.fillRect(boxX, 36, 24, 16, WHITE);
		}
		else
		{
			view_render.drawRect(boxX, 36, 24, 16, WHITE);
		}
	}

	view_render.setTextSize(1);

	for (uint16_t regIndex = 4; regIndex < MB_LHIO404_IO_Device.listRegAmount; ++regIndex)
	{
		uint8_t boxIdx = regIndex - 4;
		view_render.setTextColor(WHITE);
		view_render.setTextSize(1);
		xsprintf(str, "IN%d", regIndex - 3);
		view_render.setCursor(8 + 32 * boxIdx, 26);
		view_render.print(str);
	}

	for (uint16_t regIndex = 4; regIndex < MB_LHIO404_IO_Device.listRegAmount; ++regIndex)
	{
		uint8_t boxIdx = regIndex - 4;
		bool isActive = (boxIdx == scr_lhio404_io_device.focus_item);

		// Set text color depending on whether the box is filled (white background)
		view_render.setTextColor(isActive ? BLACK : WHITE);

		if (MB_LHIO404_IO_Device.listRegDevice[regIndex].regValue == 1)
		{
			view_render.setCursor(10 + 32 * boxIdx, 40);
			view_render.print("ON");
			BUZZER_PlaySound(BUZZER_SOUND_USB_CONNECTED);
		}
		else
		{
			view_render.setCursor(8 + 32 * boxIdx, 40);
			view_render.print("OFF");
			BUZZER_PlaySound(BUZZER_SOUND_USB_DISCONNECTED);
		}
	}

	view_render.update();
}

void scr_lhio404_io_device_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");

		timer_set(AC_TASK_DISPLAY_ID,
				  AC_DISPLAY_SHOW_MODBUS_PULL_UPDATE,
				  AC_DISPLAY_SHOW_MODBUS_PULL_INTERVAL,
				  TIMER_PERIODIC);

		timer_set(AC_TASK_DISPLAY_ID,
				  AC_DISPLAY_SHOW_MODBUS_PULL_SLEEP,
				  AC_DISPLAY_SHOW_MODBUS_PULL_SLEEP_INTERVAL,
				  TIMER_ONE_SHOT);
	}
	break;

	case AC_DISPLAY_SHOW_MODBUS_PULL_UPDATE:
	{
		updateDataModbusDevice(&MB_LHIO404_IO_Device);
	}
	break;

	case AC_DISPLAY_SHOW_MODBUS_PULL_SLEEP:
	{
		APP_DBG_SIG("AC_DISPLAY_SHOW_MODBUS_PULL_SLEEP\n");
		SCREEN_TRAN(scr_idle_handle, &scr_idle);
	}
	break;

	case AC_DISPLAY_BUTON_MODE_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_RELEASED\n");
		/* If a box is focused, toggle its output state using the mode button */
		{
			int8_t box = scr_lhio404_io_device.focus_item;
			uint16_t regIndex = 4 + box;
			if (regIndex < MB_LHIO404_IO_Device.listRegAmount)
			{
				/* Toggle local value */
				USHORT newVal = (MB_LHIO404_IO_Device.listRegDevice[regIndex].regValue == 1) ? 0 : 1;
				MB_LHIO404_IO_Device.listRegDevice[regIndex].regValue = newVal;

				/* Try writing the coil to the device */
				eMBErrorCode err = eMBMWriteSingleCoil(xMBMMaster, MB_LHIO404_IO_Device.tId, MB_LHIO404_IO_Device.listRegDevice[regIndex].regAddress, (BOOL)newVal);
				APP_DBG_SIG("Toggle coil regIndex=%d addr=%d val=%d writeErr=%d\n", regIndex,
							MB_LHIO404_IO_Device.listRegDevice[regIndex].regAddress, newVal, err);
				/* refresh view */
				view_scr_lhio404_io_device();
			}
			else
			{
				/* fallback: leave existing behavior when no focused box */
				timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_MODBUS_PULL_UPDATE);
				SCREEN_TRAN(scr_idle_handle, &scr_idle);
			}
		}
	}
	break;

	case AC_DISPLAY_BUTON_UP_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_UP_RELEASED\n");
		scr_lhio404_io_device_move_focus(1);
        BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
	}
	break;

	case AC_DISPLAY_BUTON_DOWN_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_DOWN_RELEASED\n");
		scr_lhio404_io_device_move_focus(-1);
        BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
	}
	break;

	case AC_DISPLAY_BUTON_LONG_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_LONG_MODE_PRESSED\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE);
		SCREEN_TRAN(scr_es35sw_th_sensor_handle, &scr_es35sw_th_sensor);
        BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
	}
	break;

	default:
		break;
	}
}