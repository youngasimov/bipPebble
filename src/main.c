#include <pebble.h>
	
	
#define KEY_SALDO 0
#define KEY_ERROR 1
	
Window *window;
TextLayer *header, *saldo, *footer, *updating;
static GFont font, small_font, smaller_font;
static char saldo_buffer[32];
static int tap_service_ready = 0;

static void release_tap_service(void *data){
	tap_service_ready = 1;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	if(tap_service_ready == 0){
		return;
	}
	text_layer_set_text(updating, "Actualizando...");
	DictionaryIterator *iter;
	int key = 2;
	int value = 0;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &value, sizeof(int), true);
  app_message_outbox_send();
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context){
	// Read first item
	Tuple *t = dict_read_first(iterator);
	int error = 4;
	char *s;
	// For all items
	while(t != NULL) {
		// Which key was received?
		switch(t->key) {
			case KEY_SALDO:
				s = t->value->cstring;
			break;
			case KEY_ERROR:
				error = t->value->int32;
			break;
			default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			break;
		}
		// Look for next item
		t = dict_read_next(iterator);
	}
	if(error == 0){
		snprintf(saldo_buffer, sizeof(saldo_buffer), "$%s", s);
		text_layer_set_text(header, "Tu saldo es de");
		text_layer_set_text(saldo, saldo_buffer);
		text_layer_set_text(footer, "pesos");
		text_layer_set_text(updating, "");
	}else if(error == 1){
		text_layer_set_text(updating, "Agrega tu código Bip");
	}else if(error > 1){
		text_layer_set_text(updating, "Servicio no disponible");
	}
	vibes_short_pulse();
	tap_service_ready = 0;
	app_timer_register(2000,release_tap_service,0);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
	text_layer_set_text(updating, "");
	app_timer_register(2000,release_tap_service,0);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	text_layer_set_text(updating, "");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
	// Create GFont
	font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_BOLD_40));
	small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_LIGHT_20));
	smaller_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_LIGHT_14));
	
	window_set_background_color(window,GColorBlack);
	
	  // Create header TextLayer
  header = text_layer_create(GRect(10, 30, 124, 80));
  text_layer_set_background_color(header, GColorClear);
  text_layer_set_text_color(header, GColorWhite);
	text_layer_set_font(header, small_font);
  text_layer_set_text_alignment(header, GTextAlignmentLeft);
	text_layer_set_overflow_mode(header,GTextOverflowModeWordWrap);
	//text_layer_set_text(header, "Recuperando información...");
	
  // Create saldo TextLayer
  saldo = text_layer_create(GRect(0, 55, 144, 45));
  text_layer_set_background_color(saldo, GColorClear);
  text_layer_set_text_color(saldo, GColorWhite);
	text_layer_set_font(saldo, font);
  text_layer_set_text_alignment(saldo, GTextAlignmentCenter);
	//text_layer_set_text(saldo, "");
	
	// Create footer TextLayer
  footer = text_layer_create(GRect(0, 100, 134, 68));
  text_layer_set_background_color(footer, GColorClear);
  text_layer_set_text_color(footer, GColorWhite);
  text_layer_set_font(footer, small_font);
  text_layer_set_text_alignment(footer, GTextAlignmentRight);
	text_layer_set_overflow_mode(header,GTextOverflowModeWordWrap);
	//text_layer_set_text(footer, "");
	
		// Create updating TextLayer
  updating = text_layer_create(GRect(10, 128, 134, 30));
  text_layer_set_background_color(updating, GColorClear);
  text_layer_set_text_color(updating, GColorWhite);
  text_layer_set_font(updating, smaller_font);
  text_layer_set_text_alignment(updating, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(saldo));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(updating));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(footer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(header));
	
	if (persist_exists(KEY_SALDO)) {
		text_layer_set_text(header, "Tu saldo es de");
		text_layer_set_text(saldo, saldo_buffer);
		text_layer_set_text(footer, "pesos");
		text_layer_set_text(updating, "actualizando...");
	}else{
		text_layer_set_text(header, "...");
		text_layer_set_text(updating, "actualizando...");
	}
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
		fonts_unload_custom_font(font);
		fonts_unload_custom_font(small_font);
		fonts_unload_custom_font(smaller_font);

		text_layer_destroy(header);
		text_layer_destroy(footer);
		text_layer_destroy(saldo);
		text_layer_destroy(updating);
}


static void init() {
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	accel_tap_service_subscribe(tap_handler);
	
  // Create main Window element and assign to pointer
  window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	persist_read_string(KEY_SALDO, saldo_buffer, sizeof(saldo_buffer));
	
	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);
}

void deinit(void) {
	// Destroy the window
	persist_write_string(KEY_SALDO, saldo_buffer);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}