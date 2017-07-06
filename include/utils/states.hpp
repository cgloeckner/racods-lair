#pragma once
#include <vector>
#include <memory>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace utils {

template <typename Context>
class State;

/// State machine using a given context
/**
 * An `Application` contains of a state machine with handles upcomming states
 * and handles the latest one until it was quit. Each state needs to be
 * derived from `State` using a given context. The context used for states
 * must equal the context used inside the application.
 * The application also handles the window and the entire process of polling
 * events, event propagation to the current state as well as updating and
 * drawing the current state. While the integrated mainloop, also framerate
 * limitation is done. Each second the application propagates the number of
 * frames occured during the previous second to its current state. So it's up
 * to the state to handle this or not.
 * When creating a new state, the application gives a reference of its context
 * to the child state.
 */
template <typename Context>
class Application {
  private:
	using state_ptr = std::unique_ptr<State<Context>>;

	/// RenderWindow handled by the state
	sf::RenderWindow window;

	/// Context
	Context context;

	/// State pending until next frame
	state_ptr pending;

	/// Statemachine
	std::vector<state_ptr> states;

	/// Query index of deepest visible state
	/// A state is deepest visible state if it's not transparent but
	/// all its successor states are transparent.
	/// @return index
	std::size_t getDeepest() const;

  public:
	/// Create an application
	/**
	 * The render window is completly handled by the application. The
	 * variable set of arguments is forwarded to the window's ctor.
	 * @param context reference to use as context
	 * @param args... multiple arguments forwarded to window's ctor
	 */
	template <typename... Args>
	Application(Args&&... args);

	/// Create and emplace a new state as pending
	/**
	 * S determines the type of the actual state class. Args... are
	 * multiple arguments, which are forwarded to S' ctor.
	 * @param args... multiple arguments forwarded to S' ctor
	 */
	template <typename S, typename... Args>
	void emplace(Args&&... args);

	/// Obtain an already created state as pending
	/**
	 * Moves a uniquely owned state to the application
	 * @param state reference to unique pointer to move in
	 */
	template <typename S>
	void push(std::unique_ptr<S>& ptr);

	/// Get reference to the render window
	/**
	 * @return render window handled by the application
	 */
	sf::RenderWindow& getWindow();

	/// Get const reference to the render window
	/**
	 * @return render window handled by the application
	 */
	sf::RenderWindow const& getWindow() const;

	/// Get reference to the context
	/// @return context by reference
	Context& getContext();
	
	/// Get const reference to the context
	/// @return context by const reference
	Context const & getContext() const;

	/// Query all states
	/**
	 * A vector of non-owning state pointers is returned. The last state
	 * in the vector is the active state
	 * @return vector of non-owning state pointers
	 */
	std::vector<State<Context>*> queryStates() const;

	/// Invokes the mainloop.
	/**
	 * It will terminate after the window was closed
	 */
	void run();
};

// ---------------------------------------------------------------------------

/// Base class for all states.
/**
 * Your state classes needs to be derived from this using a specific context.
 * Each state needs to implement `handle()` and `update()`, as well as the
 * interface sf::Drawable in order to be handled within an application.
 * Also `onFramerateUpdate()` can be implemented to be notified about a
 * changed framerate. It's default implementation is ignoring those updates.
 * Each state holds a reference to it's parent application as well as a
 * reference to the related context. These arguments are passed by the
 * application's factory-method `push()` without additional arguments. So all
 * arguments to push are forwarded to your state's ctor.
 * To quit the current state you can call `quit()`, so the application will
 * recognize and handle the state's proper destruction.
 */
template <typename Context>
class State : public sf::Drawable {
  private:
	/// parent application
	Application<Context>& application;

	/// determines whether the state is going to quit
	bool _quit;
	/// determines transparency of the state
	bool transparent;

  public:
	/// Base ctor
	/**
	 * @param application reference to the parent
	 */
	State(Application<Context>& application);

	/// Base dtor
	virtual ~State();

	/// Get reference to the related context
	/**
	 * @return related context
	 */
	Context& getContext();

	/// Get const-reference to the related context
	/**
	 * @return related context
	 */
	Context const& getContext() const;

	/// Get reference to the parent application
	/**
	 * @return parent application
	 */
	Application<Context>& getApplication();

	/// Get const reference to the parent application
	/**
	 * @return parent application
	 */
	Application<Context> const& getApplication() const;

	/// Causes state to be destroyed on the next frame
	void quit();

	/// Check whether the state was quit
	/**
	 * @return true if state as called `quit()`
	 */
	bool hasQuit() const;

	/// Enables/disables transparency for the state rendering
	/// If the state is transparent, the underlying state is drawn
	/// before this one is drawn. This behavior can stack over multiple
	///	states. If a state is not transparent, its opacity forces the
	/// application to render this and only this state.
	/// Note that the underlying states are not updated; their last
	/// drawable state is drawn.
	/// @param flag true makes the state transparent
	void setTransparency(bool flag);
	
	/// Query transparency
	bool isTransparent() const;

	/// Handle updates on framerate
	/**
	 * @param framerate number of frames occured during the last second
	 */
	virtual void onFramerateUpdate(float framerate);

	/// Handle input events
	/**
	 * Needs to be implemented by the derived states.
	 * @param event input event to handle
	 */
	virtual void handle(sf::Event const& event) = 0;

	/// Handle update logic
	/**
	 * Needs to be implemented by the derived states.
	 * @param elapsed time since last frame
	 */
	virtual void update(sf::Time const& elapsed) = 0;

	/// Deactivate state
	/**
	 * This method is called before a state is left. This can happen if
	 * the state was quit or another state was entered. It can be used to
	 * trigger special operations etc.
	 */
	virtual void deactivate();

	/// (Re-)activate state
	/**
	 * This method is called after a state was entered. This can happen if
	 * the state was reentered after another one was quit or when this
	 * state was pushed as current state. It can be used to trigger
	 * special operations.
	 */
	virtual void activate();
};
}

// include implementation details
#include <utils/states.inl>
