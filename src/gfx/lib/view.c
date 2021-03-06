#include "view.h"
#include <std/kheap.h>
#include <gfx/lib/shapes.h>
#include <stddef.h>

#define MAX_ELEMENTS 64

View* create_view(Rect frame) {
	View* view = kmalloc(sizeof(View));
	view->frame = frame;
	view->superview = NULL;
	view->background_color = color_make(0, 255, 0);
	view->subviews = array_m_create(MAX_ELEMENTS);
	view->labels = array_m_create(MAX_ELEMENTS);
	view->images = array_m_create(MAX_ELEMENTS);
	view->needs_redraw = 1;
	return view;
}

static View* create_title_view(Window* window) {
	Rect title_view_frame = rect_make(point_make(0, 0), size_make(window->frame.size.width, 20));
	View* title_view = create_view(title_view_frame);
	title_view->background_color = window->border_color;

	//add title label to title view
	Rect label_frame = rect_make(point_make(15, 5), title_view_frame.size);
	Label* title_label = create_label(label_frame, window->title);
	title_label->text_color = color_make(255, 255, 255);
	add_sublabel(title_view, title_label);

	return title_view;
}

static View* create_content_view(Window* window) {
	int title_height = 20;
	if (window->title_view) title_height = window->title_view->frame.size.height;

	Rect inner_frame = rect_make(point_make((window->border_width * 2), title_height), size_make(window->frame.size.width - (window->border_width * 4), window->frame.size.height - title_height - (window->border_width * 2)));
	View* content_view = create_view(inner_frame);
	content_view->background_color = color_make(255, 255, 255);
	
	return content_view;
}

Window* create_window(Rect frame) {
	Window* window = kmalloc(sizeof(Window));
	window->size = frame.size;
	window->frame = frame;
	window->border_color = color_make(0, 0, 255);
	window->border_width = 1;
	window->subviews = array_m_create(MAX_ELEMENTS);
	window->title = "Window";

	window->title_view = create_title_view(window);
	window->content_view = create_content_view(window);

	window->needs_redraw = 1;
		
	return window;
}

Label* create_label(Rect frame, char* text) {
	Label* label = kmalloc(sizeof(Label));
	label->frame = frame;
	label->text = text;
	label->superview = NULL;
	label->text_color = color_make(0, 0, 0);
	label->needs_redraw = 1;
	return label;
}

Image* create_image(Rect frame, uint32_t* bitmap) {
	Image* image = kmalloc(sizeof(Image));
	image->frame = frame;
	image->bitmap = bitmap;
	image->needs_redraw = 1;
	return image;
}

void mark_needs_redraw(View* view) {
	//if this view has already been marked, quit
	if (view->needs_redraw) return;
	view->needs_redraw = 1;
	if (view->superview) {
		mark_needs_redraw(view->superview);
	}
}

void add_sublabel(View* view, Label* label) {
	array_m_insert(view->labels, label);
	label->superview = view;
	label->needs_redraw = 1;
	mark_needs_redraw(view);
}

void remove_sublabel(View* view, Label* label) {
	array_m_remove(view->labels, array_m_index(view->labels, label));
	label->superview = NULL;
	label->needs_redraw = 1;
	mark_needs_redraw(view);
}

void add_subimage(View* view, Image* image) {
	array_m_insert(view->images, image);
	image->superview = view;
	image->needs_redraw = 1;
	mark_needs_redraw(view);
}

void remove_subimage(View* view, Image* image) {
	array_m_remove(view->images, array_m_index(view->images, image));
	image->superview = NULL;
	image->needs_redraw = 1;
	mark_needs_redraw(view);
}

void add_subview(View* view, View* subview) {
	array_m_insert(view->subviews, subview);
	subview->superview = view;
	mark_needs_redraw(view);
}

void remove_subview(View* view, View* subview) {
	array_m_remove(view->subviews, array_m_index(view->subviews, subview));
	subview->superview = NULL;
	subview->needs_redraw = 1;
	mark_needs_redraw(view);
}

void set_background_color(View* view, Color color) {
	view->background_color = color;
	mark_needs_redraw(view);
}

void add_subwindow(Window* window, Window* subwindow) {
	array_m_insert(window->subviews, subwindow);
	subwindow->superview = window;
	mark_needs_redraw(window);
}

void remove_subwindow(Window* window, Window* subwindow) {
	array_m_remove(window->subviews, array_m_index(window->subviews, subwindow));
	subwindow->superview = NULL;
	mark_needs_redraw(window);
}

void set_border_width(Window* window, int width) {
	window->border_width = width;
	mark_needs_redraw(window);
}

void set_frame(View* view, Rect frame) {
	view->frame = frame;
	mark_needs_redraw(view);
}
