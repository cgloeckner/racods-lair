#pragma once
#include <functional>
#include <string>
#include <list>
#include <memory>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <utils/input_mapper.hpp>

namespace utils {

extern unsigned int const MENU_COOLDOWN;

/// Implemented menu actions
enum class MenuAction: std::size_t {
	NavigatePrev,
	NavigateNext,
	Activate,
	AlternatePrev,
	AlternateNext
};

struct MenuActionHash {
	std::size_t operator()(MenuAction action) const {
		return static_cast<std::size_t>(action);
	}
};

/// Base class for widgets
class Widget : public sf::Drawable {

  protected:
	using Callback = std::function<void()>;

	/// Describes whether widget is visible
	bool visible;
	
  public:
	/// Create a blank widget
	Widget();

	virtual ~Widget();

	/// Change visibility
	/**
	 * @param visible true is used for visibility, false for invisibility
	 */
	void setVisible(bool visible);

	/// Query visibility
	/**
	 * @return true if widget is visible
	 */
	bool isVisible() const;

	/// Handles a `MenuAction`
	/// @param action MenuAction to handle
	virtual void handle(MenuAction action) = 0;

	/// Handle typing a unicode character
	/// @param unicode character
	virtual void handle(sf::Uint32 unicode);

	/// Set position
	/**
	 * Whether the position is used as topleft, center or else is up to
	 * the actual implementation.
	 * @param position to use
	 */
	virtual void setPosition(sf::Vector2f const& pos) = 0;

	/// Set widget focus
	/**
	 * @param focus true if widget is now focused, false if focus was lost
	 */
	virtual void setFocus(bool focus) = 0;
	
	/// Update widget's custom state
	virtual void update(sf::Time const & elapsed);
};

// ---------------------------------------------------------------------------

/// Blueprint for all kind of triggerable widgets
class Button : public Widget {
  protected:
	/// React on activation
	virtual void onActivate();

  public:
	/// Callbacks for activate
	Callback activate;

	void handle(MenuAction action) override;
};

// ---------------------------------------------------------------------------

/// Blueprint for all kind of selection widgets
/**
 * A Select contains multiple strings and holds an index to one of these items.
 */
class Select : public Widget, public std::vector<std::string> {

  private:
	/// Index to an item
	std::size_t index;

  protected:
	/// React on activation
	virtual void onActivate();
	
	/// React on changing
	virtual void onChanged() = 0;
	
	/// React on update
	virtual void onUpdate() = 0;

  public:
	/// Callbacks for activate
	Callback activate;
	/// Callback for change
	Callback change;

	/// Create an empty select widget
	Select();

	void handle(MenuAction action) override;

	/// Set the current index
	/**
	 * Using an index which is out of bounce is undefined behavior. After
	 * changing the index, onChanged() is called.
	 * @param index to use from now on
	 */
	void setIndex(std::size_t index);

	/// Get the current index
	/**
	 * If the container is empty, the return value of this function is
	 * invalid.
	 * @return current index
	 */
	std::size_t getIndex() const;
};

// ---------------------------------------------------------------------------

class Input : public Widget {

  protected:
	virtual void onTyping();
	std::size_t max_len;

	bool isAllowed(sf::Uint32 unicode) const;
	
	virtual void onType() = 0;
	virtual void onUndo() = 0;

  public:
	std::vector<sf::Uint32> whitelist, blacklist;
	std::function<void(sf::Uint32, bool)> typing;

	Input();
	
	virtual sf::String getContent() const = 0;
	virtual void setContent(sf::String const& string) = 0;

	virtual void handle(utils::MenuAction action) override;
	virtual void handle(sf::Uint32 unicode) override;
	
	/// @param max_len Maximum length for input content. 0: no limit
	void setMaxLength(std::size_t max_len);
	std::size_t getMaxLength() const;
};

// ---------------------------------------------------------------------------

/// Menu context / container
/**
 * Owns multiple widgets which are created using the factory method `create`.
 * All widgets are updated and drawn automatically when the menu is updated
 * and drawn.
 * You need to bind input actions to the predefined menu actions to enable
 * proper usage.
 */
template <typename T>
class Menu : public sf::Drawable {
  private:
	using pair = std::pair<T, std::unique_ptr<Widget>>;
	// note: list guarantees order and offers bidirectional iterator
	using container = std::list<pair>;

	/// Owned widgets
	container widgets;

	/// Currently focused widget
	T focus;

	/// Received unicode symbols since last update
	std::vector<sf::Uint32> unicodes;

	/// Input bindings
	InputMapper input;
	std::unordered_map<MenuAction, std::vector<InputAction>, MenuActionHash> binding;

	/// Cooldown (especially necessary for gamepad input!)
	sf::Time cooldown;

	/// Test whether menu action is currently active
	bool isActive(MenuAction action);

	/// Query iterator of a given key
	/**
	 * @param key of the widget
	 * @return bidirectional iterator
	 */
	typename container::iterator at(T key);
	
	/// Query const iterator of a given key
	/**
	 * @param key of the widget
	 * @return bidirectional const iterator
	 */
	typename container::const_iterator at(T key) const;

	/// Changes focus to given index
	/**
	 * If the target is invisible, the next widget is chosen. If no widget
	 * is available, the method will return without an effect.
	 * @param index to focus
	 * @param forward determines skip direction if widget invisible
	 */
	void changeFocus(T key, bool forward = true);

	/// Draw method used for rendering
	/**
	 * @param target to render at
	 * @param states to use while rendering
	 */
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
	/// Create a blank menu
	Menu();

	/// Factory method to create a widget
	/**
	 * All arguments are forwarded to the actual ctor of the widget class
	 * `W`. The returned widget is automatically managed and owned by the
	 * menu.
	 * @param key to identify the widget with
	 * @param args... forwarded to ctor of `W`
	 * @return reference to the created widget
	 */
	template <typename W, typename... Args>
	W& acquire(T key, Args&&... args);

	/// Query a widget by its key
	/**
	 * @pre The widgets needs to exist and be an instance of type `W`
	 * @param key to identify the widget with
	 * @return reference to the queried widget
	 */
	template <typename W>
	W& query(T key);
	
	/// Query a const widget by its key
	/**
	 * @pre The widgets needs to exist and be an instance of type `W`
	 * @param key to identify the widget with
	 * @return const reference to the queried widget
	 */
	template <typename W>
	W const & query(T key) const;

	/// Release a widget
	/**
	 * This will destroy and release the described widget. All external
	 * references to the widget are invalidated
	 * @pre The widgets needs to exist
	 * @param key to identify the widget with
	 */
	void release(T key);

	/// Focus the given widget.
	/**
	 * Sets focus of the container to the given widget, which needs to be
	 * created by the container in the first place.
	 * @param key to identify the widget with
	 */
	void setFocus(T key);

	/// Focus the given widget.
	/**
	 * Sets focus of the container to the given widget, which needs to be
	 * created by the container in the first place.
	 * @param widget to focus
	 */
	template <typename W>
	void setFocus(W const& widget);

	/// Query key of currently focused widget
	/**
	 * @pre Menu needs to contain at least one visible widget
	 * @return key of widget
	 */
	T queryFocus() const;

	void clear(MenuAction const & action);

	/// Binds an input action to a predefined menu action
	/**
	 * See `MenuAction` for available actions. This method is used to
	 * define the conditions necessary for menu action to occur.
	 * @param action menu action to bind to
	 * @param input input action to bind with
	 */
	void bind(MenuAction const & action, InputAction const & input);

	/// Propagate an event to the menu
	/**
	 * Propagation is necessary to enable input handling by the menu.
	 * @param event propagated to the container
	 */
	void handle(sf::Event const& event);

	/// Update container's behavior
	/// Up to previously propagated events, the container will evaluate
	/// input to change focus or operate on the currently focused widget.
	///
	/// @param elapsed Time since last update
	void update(sf::Time const & elapsed);
};

}  // ::sfext

// include implementation details
#include <utils/menu.inl>
