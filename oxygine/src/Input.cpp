#include "Input.h"
#include "RootActor.h"
#include "core/logging.h"

namespace oxygine
{
	Input Input::instance;

	const PointerState *TouchEvent::getPointer() const
	{
		return Input::instance.getTouchByIndex(index);
	}

	void Input::sendPointerButtonEvent(MouseButton button, int x, int y, int type, PointerState *ps)
	{
		//log::message("sendPointerButtonEvent %d %d", x, y);
		/*
		const Rect &vp = RootActor::instance->getViewport();
		x -= vp.pos.x;
		y -= vp.pos.y;

		Vector2 p(x, y);
		Point ds = core::getDisplaySize();
		p.x /= vp.size.x;
		p.y /= vp.size.y;

		p.x *= RootActor::instance->getWidth();
		p.y *= RootActor::instance->getHeight();
		*/

		Vector2 p((float)x, (float)y);

		TouchEvent me(type, true, p);
		me.index = ps->getIndex();
		me.mouseButton = button;
		
		if (type == TouchEvent::TOUCH_DOWN)
			ps->_isPressed[button] = true;
		else
		if (type == TouchEvent::TOUCH_UP)
			ps->_isPressed[button] = false;

		ps->_position = Point(x, y);

		RootActor::instance->handleEvent(&me);
	}

	void Input::sendPointerMotionEvent(int x, int y, PointerState *ps)
	{
		TouchEvent me(TouchEvent::MOVE, true, Vector2((float)x, (float)y));
		me.index = ps->getIndex();
		ps->_position = Point(x, y);

		RootActor::instance->handleEvent(&me);
	}

	void Input::sendPointerWheelEvent(int scroll, PointerState *ps)
	{
		TouchEvent me(scroll > 0 ? TouchEvent::WHEEL_UP : TouchEvent::WHEEL_DOWN, true);
		me.index = ps->getIndex();

		ps->_position = Point(0, 0);

		RootActor::instance->handleEvent(&me);
	}


	Input::Input()
	{
		_pointerMouse.init(MAX_TOUCHES + 1);
		for (int i = 0; i < MAX_TOUCHES; ++i)
			_pointers[i].init(i + 1);

		memset(_ids, 0, sizeof(_ids));
	}

	Input::~Input()
	{

	}
	
	void Input::cleanup()
	{
	}

	PointerState *Input::getTouchByIndex(int index)
	{
		if (index == MAX_TOUCHES + 1)
			return &_pointerMouse;
		index -= 1;
		OX_ASSERT(index >= 0 && index < MAX_TOUCHES);
		index = min(max(index, 0), MAX_TOUCHES);
		return &_pointers[index];
	}

#ifndef __S3E__
	int Input::touchID2index(int id)
	{
		id += 1;//id could be = 0 ?
		for (int i = 0; i < MAX_TOUCHES; ++i)
		{
			int &d = _ids[i];
			int index = i + 1;
			if (d == 0)
			{
				d = id;
				return index;
			}

			if (d == id)			
				return index;
		}
		OX_ASSERT("can't find touch id");
		return 0;
	}

	PointerState *Input::getTouchByID(int id)
	{
		return getTouchByIndex(touchID2index(id));
	}
#endif
}