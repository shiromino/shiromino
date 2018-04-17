/*
    SGUIL - Simple GUI Layer version 0.1.0 for SDL2 >=2.0.5
*/

#ifndef _SGUIL_hpp
#define _SGUIL_hpp

#define SGUIL_VERSION_STR "0.1.0_01"

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <utility>
#include <memory>
#include <functional>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <SDL2/SDL.h>

#define GUI_TEXT_ALIGN_CENTER   0x0002
#define GUI_TEXT_OUTLINE        0x0004
#define GUI_TEXT_SHADOW         0x0008
#define GUI_TEXT_HIGHLIGHT      0x0010
#define GUI_TEXT_ALIGN_RIGHT    0x0020

#define GUI_RGBA_DEFAULT            0xFFFFFFFF
#define GUI_RGBA_OUTLINE_DEFAULT    0x000000FF

#define GUI_WINDOW_CALLBACK_NONE    NULL

typedef unsigned char byte_t;
typedef uint32_t rgba_t;
typedef int optionID_t;

extern SDL_Renderer *Gui_SDL_Renderer;
extern SDL_Texture *Gui_ThemeTexture;

bool Gui_Init(SDL_Renderer *, const char *);

struct BitFont
// sheets should be 32w x 4h characters in dimensions
{
    BitFont() : sheet(NULL), outlineSheet(NULL), charW(0), charH(0) {isValid = false;}
    BitFont(const char *, const char *, unsigned int, unsigned int);
    ~BitFont();

    bool isValid;

    SDL_Texture *sheet;
    SDL_Texture *outlineSheet;
    unsigned int charW;
    unsigned int charH;
};

enum class enumAlignment
{
    left, right, center, justified
};

struct TextFormat
{
    TextFormat()
        : TextFormat(0, GUI_RGBA_DEFAULT, GUI_RGBA_OUTLINE_DEFAULT) {}

    TextFormat(rgba_t rgba, rgba_t rgbaOutline)
        : TextFormat(0, rgba, rgbaOutline) {}

    TextFormat(unsigned int flags, rgba_t rgba, rgba_t rgbaOutline)
        : rgba(rgba), rgbaOutline(rgbaOutline)
    {
        rgbaHighlight = 0x8080FFFF;

        highlight = flags & GUI_TEXT_HIGHLIGHT;
        outline = flags & GUI_TEXT_OUTLINE;
        shadow = flags & GUI_TEXT_SHADOW;
        sizeMult = 1.0;
        lineSpacing = 1.0;
        alignment = enumAlignment::left;
        wrapLen = 0;

        if(flags & GUI_TEXT_ALIGN_RIGHT)
        {
            alignment = enumAlignment::right;
        } else if(flags & GUI_TEXT_ALIGN_CENTER)
        {
            alignment = enumAlignment::center;
        }
    }

    rgba_t rgba;
    rgba_t rgbaOutline;
    rgba_t rgbaHighlight;

    bool highlight;
    bool outline;
    bool shadow;

    float sizeMult;
    float lineSpacing;
    enumAlignment alignment;
    unsigned int wrapLen;
};

// extern std::map<std::string, BitFont> Gui_MyBitFonts;

enum enumGuiEventType
{
mouse_event = 0x1000,
    mouse_clicked,
    mouse_released,
    mouse_dragged,
    mouse_hovered_onto,
    mouse_hovered_off,
    mouse_moved,
key_event = 0x2000,
    key_pressed,
    key_released,
textinput_event = 0x2FFF,
joy_event = 0x4000,
    joybutton_pressed,
    joybutton_released
};

struct GuiMouseEvent
{
    GuiMouseEvent() {x = 0; y = 0;}
    GuiMouseEvent(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

struct GuiMouseButtonEvent
{
    GuiMouseButtonEvent() {x = 0; y = 0; button = SDL_BUTTON_LEFT;}
    GuiMouseButtonEvent(int x, int y, Uint8 button) : x(x), y(y), button(button) {}

    int x;
    int y;
    Uint8 button;
};

struct GuiKeyEvent
{
    GuiKeyEvent() {key = SDLK_UNKNOWN;}
    GuiKeyEvent(SDL_Keycode kc) : key(kc) {}

    SDL_Keycode key;
};

struct GuiTextInputEvent
{
    GuiTextInputEvent() : text() {}
    GuiTextInputEvent(std::string s) : text(s) {}

    std::string text;
};

struct GuiEvent
{
    GuiEvent(enumGuiEventType type, int x, int y)
        : type(type), mouseEvent(x, y)
    {
        switch(type)
        {
            case mouse_clicked:
                mouseClickedEvent = &mouseEvent;
                break;
            case mouse_released:
                mouseReleasedEvent = &mouseEvent;
                break;
            case mouse_dragged:
                mouseDraggedEvent = &mouseEvent;
                break;
            case mouse_hovered_onto:
                mouseHoveredOntoEvent = &mouseEvent;
                break;
            case mouse_hovered_off:
                mouseHoveredOffEvent = &mouseEvent;
                break;
            case mouse_moved:
                mouseMovedEvent = &mouseEvent;
                break;
            default:
                break;
        }
    }

    GuiEvent(enumGuiEventType type, SDL_Keycode kc)
        : type(type), keyEvent(kc)
    {
        switch(type)
        {
            case key_pressed:
                keyPressedEvent = &keyEvent;
                break;
            case key_released:
                keyReleasedEvent = &keyEvent;
                break;
            default:
                break;
        }
    }

    GuiEvent(enumGuiEventType type, std::string s)
        : type(type), typingEvent(s)
    {
        textInputEvent = &typingEvent;
    }

    enumGuiEventType type;

    union
    {
        GuiMouseEvent *mouseClickedEvent;
        GuiMouseEvent *mouseReleasedEvent;
        GuiMouseEvent *mouseDraggedEvent;
        GuiMouseEvent *mouseHoveredOntoEvent;
        GuiMouseEvent *mouseHoveredOffEvent;
        GuiMouseEvent *mouseMovedEvent;

        GuiKeyEvent *keyPressedEvent;
        GuiKeyEvent *keyReleasedEvent;

        GuiTextInputEvent *textInputEvent;
    };

private:
    GuiMouseEvent mouseEvent;
    GuiKeyEvent keyEvent;
    GuiTextInputEvent typingEvent;
};

template<typename T>
struct GuiEventHook
{
    GuiEventHook(std::function<void(T&, GuiEvent&)> callback, enumGuiEventType type)
        : eventType(type), callback(callback) {}

    enumGuiEventType eventType;
    std::function<void(T&, GuiEvent&)> callback;
};

template<typename T>
struct GuiRenderHook
{
    GuiRenderHook(std::function<void(T&)> callback) : callback(callback) {}

    std::function<void(T&)> callback;
};

class GuiWindow;

class GuiElement
// base class for all gui objects
{
public:
    GuiElement() {}
    virtual ~GuiElement() {};

    virtual void draw() = 0;
    void prepareRenderTarget(bool);
    void setWindow(GuiWindow& w)
    {
        containingWindow = &w;
    }
    GuiWindow *getWindow()
    {
        return containingWindow;
    }

protected:
    GuiWindow *containingWindow;
    // relativeDestRect's x, y fields are relative to container's canvas texture
    SDL_Rect relativeDestRect;
};

class GuiText : public GuiElement
// simple text box with (probably) unchanging text, can be used for explanations and tooltips
{
public:
    GuiText(std::string, BitFont&, SDL_Rect&);
    ~GuiText() {}

    void draw();
    void addEventHook(std::function<void(GuiText&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiText&)>);

private:
    std::vector<GuiEventHook<GuiText>> eventHooks;
    std::vector<GuiRenderHook<GuiText>> renderHooks;

    std::string text;
    std::vector<std::pair<int, int>> textPositionalValues;
    TextFormat fmt;
    BitFont& font;

    bool updatePositionalValues;
};

class GuiInteractable : public GuiElement
// base class for all gui elements than can be interacted with
{
public:
    // canInteractAt: checks if the given mouse X and Y are inside the element's rectangle,
    // and if the element is enabled. if so, either perform some action or let the user do so
    virtual bool canInteractAt(int, int);
    virtual void mouseClicked(int, int) = 0;
    virtual void mouseDragged(int, int) = 0;
    virtual void mouseReleased(int, int) = 0;
    virtual void keyPressed(SDL_Keycode) = 0;
    virtual void textInput(std::string) {}

    virtual void handleEvent(GuiEvent&);

    bool enabled;
    bool canHoldKeyboardFocus;
    int ID;

    // these are set by either the user or the parent window if there is one
    bool selected;
    bool hasKeyboardFocus;

protected:
    std::string displayString;
    std::vector<std::pair<int, int>> displayStringPositionalValues;
    SDL_Texture *displayTexture;
    bool updateDisplayStringPVs;
};

class BindableVariable
{
public:
	BindableVariable(const std::string& name) : name_(name) {}
	virtual ~BindableVariable() {}

	std::string name() const { return name_; }
	virtual void set(const std::string& val) = 0;
	virtual std::string get() const = 0;

	void addObserver(std::function<void(BindableVariable *)> ob) { observers.push_back(ob); }

protected:
	void valueChanged()
	{
		for(auto ob : observers)
        {
            ob(this);
        }
	}

private:
	const std::string name_;
    std::vector<std::function<void(BindableVariable *)>> observers;
};

class BindableString : public BindableVariable
{
public:
	BindableString(const std::string& name) : BindableVariable(name) {}
	virtual void set(const std::string& val) override
	{
		if(val != value)
		{
			value = val;
			valueChanged();
		}
	}

	virtual std::string get() const override { return value; }

private:
	std::string value;
};

class BindableInt : public BindableVariable
{
public:
	// min is inclusive, max is exclusive
	BindableInt(const std::string& name, int64_t min, int64_t max)
		: BindableVariable(name), min(min), max(max), value(0) {}

	virtual void set(const std::string& val) override { setInt(stoll(val)); }
	virtual std::string get() const { return std::to_string(getInt()); }


	void setInt(int64_t&& val)
	{
		if(min <= val && val < max)
		{
			if(val != value)
			{
				value = val;
				valueChanged();
			}
		}
	}

	int64_t getInt() const { return value; }

	std::pair<int64_t, int64_t> getRange() const { return { min, max }; }

protected:
	int64_t value;
	const int64_t min;
	const int64_t max;
};

class BindableEnumeration : public BindableInt
{
public:
	BindableEnumeration(const std::string& name, std::vector<std::string>&& values)
		: BindableInt(name, 0, values.size()), values(std::move(values)) {}

	virtual void set(const std::string& val) override
	{
		auto it = std::find(values.begin(), values.end(), val);
		if(it != values.end())
		{
			auto newval = it - values.begin();
			if(newval != value)
			{
				value = newval;
				valueChanged();
			}
		}
	};

	virtual std::string get() const
	{
		return displayNameForValue(value);
	}

	std::string displayNameForValue(int val) const
	{
		assert(val >= 0 && val < values.size());
		return values[val];
	};

private:
	std::vector<std::string> values;
};

class BindableFloat : public BindableVariable
{
public:
	BindableFloat(const std::string& name, long double min, long double max)
		: BindableVariable(name), min(min), max(max), value(0.0) {}

	virtual void set(const std::string& val) override { setFloat(stold(val)); }
	virtual std::string get() const { return std::to_string(getFloat()); }


	void setFloat(long double&& val)
	{
		if(min <= val && val < max)
		{
			if(val != value)
			{
				value = val;
				valueChanged();
			}
		}
	}

	long double getFloat() const { return value; }

	std::pair<long double, long double> getRange() const { return { min, max }; }

protected:
	long double value;
	const long double min;
	const long double max;
};

class BindableVariables
{
public:
	void add(std::unique_ptr<BindableVariable> var)
	{
		assert(!find(var->name()));
		vars.emplace_back(std::move(var));
	};

	BindableVariable* find(const std::string& name)
	{
        if(vars.size() == 0)
        {
            return NULL;
        }

		auto it = std::find_if(vars.begin(), vars.end(), [&name](const auto& p){ return p->name() == name; });
		if(it != vars.end())
        {
			return it->get();
        }

        return NULL;
	};

private:
	std::vector<std::unique_ptr<BindableVariable>> vars;
};

enum enumOptionAccess
{
    no_access           = 0,
    use_callback        = 1 << 0,
    random_access_copy  = 1 << 1,
    random_access       = 1 << 2
};

inline enumOptionAccess operator | (enumOptionAccess a, enumOptionAccess b)
{ return static_cast<enumOptionAccess>(static_cast<int>(a) | static_cast<int>(b)); }

inline enumOptionAccess operator |= (enumOptionAccess a, enumOptionAccess b)
{ return a = a | b; }

class GuiOptionInteractable : public GuiInteractable
{
protected:
    BindableVariable *var;
    //std::function<int(optionID_t, T)> valueUpdateCallback;
    //optionID_t optionID;

    //std::vector<T> valueOptions;
    //int choice;
    //T value;

    //enumOptionAccess accessType;
    //T *accessPtr;
};

// template<typename T>
class GuiTextField : public GuiOptionInteractable
// text box which the user can send input to
{
public:
    GuiTextField(int, BindableString *, BitFont&, SDL_Rect);
    GuiTextField(int, BindableString *, std::string, BitFont&, SDL_Rect);
    // GuiTextField(int, optionID_t, std::string, BitFont&, SDL_Rect, std::function<int(optionID_t, std::string)>, std::string *, bool);
    // GuiTextField(const GuiTextField&);
    ~GuiTextField();

    void setTextFormat(TextFormat&);

    void draw();
    void addEventHook(std::function<void(GuiTextField&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiTextField&)>);

    void handleEvent(GuiEvent&);

    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);
    void textInput(std::string);

    unsigned int getPositionUnderMouse(int, int);

    unsigned int shiftCursor(int);
    bool textInsert(std::string);
    void textDelete(); // delete selection
    void textDelete(unsigned int, unsigned int);
    std::string textCut();
    std::string textCopy();

protected:
    std::vector<GuiEventHook<GuiTextField>> eventHooks;
    std::vector<GuiRenderHook<GuiTextField>> renderHooks;

    std::vector<std::pair<int, int>> textPositionalValues;
    std::string value;
    TextFormat fmt;
    BitFont& font;
    uint64_t lastEventTime;
    bool typing;
    unsigned int cursor;
    unsigned int scrollPosX;
    unsigned int scrollPosY;

    unsigned int selectionStart;
    unsigned int selectionEnd;

    bool updatePositionalValues;
    bool enableNewline;
    bool horizontalScroll;
    bool verticalScroll;
};

class GuiButton : public GuiInteractable
// clickable, causes a direct action to take place, can also be scrolled through with arrow keys or similar
{
public:
    GuiButton(int, SDL_Rect, std::string, BitFont&);
    // GuiButton(const GuiButton&);
    ~GuiButton();

    void draw();
    void addEventHook(std::function<void(GuiButton&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiButton&)>);
    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);

private:
    std::vector<GuiEventHook<GuiButton>> eventHooks;
    std::vector<GuiRenderHook<GuiButton>> renderHooks;

    BitFont &font;
};
/*
template<typename T>
class GuiOptionButton : public GuiOptionInteractable<T>
{
public:
    GuiOptionButton(int, optionID_t, SDL_Rect, std::string, BitFont&);
    ~GuiOptionButton();

    void draw();
    void addEventHook(std::function<void(GuiOptionButton&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiOptionButton&)>);
    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);

private:
    std::vector<GuiEventHook<GuiButton>> eventHooks;
    std::vector<GuiRenderHook<GuiButton>> renderHooks;

    BitFont &font;
};
*//*
template<typename T>
class GuiDropDownList : public GuiOptionInteractable
// self-explanatory (list of T with associated strings); this->value should always equal entry[selection]
{
public:
    GuiDropDownList(int, optionID_t, SDL_Rect);
    ~GuiDropDownList();

    void draw();
    void addEventHook(std::function<void(GuiDropDownList&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiDropDownList&)>);
    bool canInteractAt(int, int);
    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);

    bool addEntry(T, std::string);

private:
    std::vector<std::pair<T, std::string>> entry;
    unsigned int selection;
    bool menuShown;
};
*/

class GuiWindow
// essentially a container for GuiElement with relative positioning, possibly with -oX controls like in a WM
{
public:
    GuiWindow(std::string, BitFont&, std::function<void(GuiInteractable&)>, SDL_Rect&);
    ~GuiWindow();

    void draw();
    void addEventHook(std::function<void(GuiWindow&, GuiEvent&)>, enumGuiEventType);
    void addRenderHook(std::function<void(GuiWindow&)>);

    // void addText(GuiText&);
    // void addTextField(GuiTextField&);
    // void addButton(GuiButton&);

    void addElement(GuiElement *);
    void addControlElement(GuiInteractable *);

    void handleSDLEvent(SDL_Event&);
    GuiInteractable *getControlElementAt(int, int);

    void moveRect(int, int);

    void mouseMoved(int, int);
    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);
    void textInput(std::string);

    SDL_Texture *canvas;
    SDL_Rect destRect;

private:
    std::vector<GuiElement *> elements;
    std::vector<GuiInteractable *> controlList;
    int keyboardFocus;
    int controlSelection;
    bool selectingByMouse;

    std::vector<GuiEventHook<GuiWindow>> eventHooks;
    std::vector<GuiRenderHook<GuiWindow>> renderHooks;
    std::function<void(GuiInteractable&)> interactionEventCallback;

    rgba_t rgbaBackground;
    rgba_t rgbaTitleBar;

    std::string title;
    std::vector<std::pair<int, int>> titlePositionalValues;
    BitFont& titleFont;
    int titleBarHeight;

    bool showTitleBar;
    bool updateTitlePositionalValues;
    bool moveable;

    bool useExtWindowX;
    bool useExtWindowY;
    bool moving;
    int initialX;
    int initialY;
    int moveBeginX;
    int moveBeginY;
};

/*
class GuiPanel : public GuiWindow
{

};
*/

inline byte_t rgba_R(rgba_t rgba)
{
    return (rgba & 0xFF000000) / 0x1000000;
}

inline byte_t rgba_G(rgba_t rgba)
{
    return (rgba & 0x00FF0000) / 0x0010000;
}

inline byte_t rgba_B(rgba_t rgba)
{
    return (rgba & 0x0000FF00) / 0x0000100;
}

inline byte_t rgba_A(rgba_t rgba)
{
    return (rgba & 0x000000FF);
}

inline void GuiElement::prepareRenderTarget(bool isFinalCopy) {
    if(this->containingWindow)
    {
        SDL_SetRenderTarget(Gui_SDL_Renderer, this->containingWindow->canvas);
    } else
    {
        SDL_SetRenderTarget(Gui_SDL_Renderer, NULL);
    }
}

inline void Gui_SetTextureRGBA(SDL_Texture *tex, rgba_t rgba)
{
    SDL_SetTextureColorMod(tex, rgba_R(rgba), rgba_G(rgba), rgba_B(rgba));
    SDL_SetTextureAlphaMod(tex, rgba_A(rgba));
}

void Gui_DrawBorder(SDL_Rect&, int, rgba_t);

void Gui_GenerateTextPositionalValues(std::string&, TextFormat *, BitFont&, SDL_Rect&, std::vector<std::pair<int, int>>&, bool, bool);
void Gui_GenerateTextPositionalValuesPartial(std::string&, unsigned int, unsigned int, TextFormat *, BitFont&, SDL_Rect&, std::vector<std::pair<int, int>>&, bool, bool);

void Gui_DrawText_PV(std::string, TextFormat *, BitFont&, std::vector<std::pair<int, int>>&, unsigned int, unsigned int);
void Gui_DrawTextPartial_PV(std::string, unsigned int, unsigned int, TextFormat *, BitFont&, std::vector<std::pair<int, int>>&, unsigned int, unsigned int);
void Gui_DrawText(std::string, TextFormat *, BitFont&, SDL_Rect&);
void Gui_DrawTextPartial(std::string, unsigned int, unsigned int, TextFormat *, BitFont&, SDL_Rect&);

#endif
