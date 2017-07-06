#pragma once
#include <thread>
#include <atomic>
#include <Thor/Graphics.hpp>

#include <state/common.hpp>

namespace state {

class LoadThreadState
	: public State {
  private:
	std::thread loader;
	std::atomic<bool> loaded;
	bool finished;
	
	virtual void load() = 0;
	virtual void postload() = 0;
	
  protected:
	void start();
	
  public:
	LoadThreadState(App& app);
	~LoadThreadState();
	
	virtual void update(sf::Time const& elapsed) override;
};

} // ::state
