#include <pebble.h>
	
	
#define KEY_SALDO 0
#define KEY_ERROR 1
	
Window *window;
TextLayer *header, *saldo, *footer, *updating;
static PropertyAnimation *saldo_animation;
static GFont font, small_font, smaller_font;
static char saldo_buffer[32];
static int tap_service_ready = 0, animation_stage = 0;

static void trigger_animation();

void animation_started(Animation *animation, void *data) {
	if(animation_stage == 1){
		text_layer_set_text(saldo, saldo_buffer);
	}
}

void animation_stopped(Animation *animation, bool finished, void *data) {
	property_animation_destroy(saldo_animation);
	if(animation_stage == 0){
		layer_set_frame(text_layer_get_layer(saldo),GRect(144, 55, 144, 45));
		animation_stage++;
		trigger_animation();
	}else{
		animation_stage = 0;
	}
}

static void trigger_animation(){
	GRect from_frame, to_frame;
	if(animation_stage == 0){
		from_frame = layer_get_frame(text_layer_get_layer(saldo));
  	to_frame = GRect(-144, 55, 144, 45);
	}else if(animation_stage == 1){
		from_frame = layer_get_frame(text_layer_get_layer(saldo));
  	to_frame = GRect(0, 55, 144, 45);
	}
	
	saldo_animation = property_animation_create_layer_frame(text_layer_get_layer(saldo), &from_frame, &to_frame);
	
	// You may set handlers to listen for the start and stop events
  animation_set_handlers((Animation*) saldo_animation, (AnimationHandlers) {
    .started = (AnimationStartedHandler) animation_started,
    .stopped = (AnimationStoppedHandler) animation_stopped,
  }, NULL);
	
	animation_schedule((Animation*) saldo_animation);
}

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
	tap_service_ready = 0;
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
		text_layer_set_text(footer, "pesos");
		text_layer_set_text(updating, "");
		vibes_short_pulse();
		trigger_animation();
		
	}else if(error == 1){
		text_layer_set_text(updating, "Agrega tu código Bip");
	}else if(error > 1){
		text_layer_set_text(updating, "Servicio no disponible");
	}
	tap_service_ready = 0;
	app_timer_register(2000,release_tap_service,0);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(updating, "");
	}else{
			text_layer_set_text(updating, "Desconectado");
	}
	tap_service_ready = 1;
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	tap_service_ready = 1;
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(updating, "");
	}else{
			text_layer_set_text(updating, "Desconectado");
	}
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
	}else{
		text_layer_set_text(header, "...");
	}
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(updating, "Actualizando...");
	}else{
		text_layer_set_text(updating, "Desconectado");
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
	animation_unschedule_all();
	// Destroy the window
	persist_write_string(KEY_SALDO, saldo_buffer);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}