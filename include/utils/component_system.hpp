#pragma once
#include <vector>

namespace utils {

template <typename Id>
class IdManager {
  private:
	std::size_t const n;
	Id i;
	std::vector<Id> recent, unused;

  public:
	IdManager(std::size_t n=100u);

	Id acquire();
	void release(Id id);
	
	void cleanup();
	
	void reset();
};

// ---------------------------------------------------------------------------

template <typename Id>
struct BaseSystem {
	~BaseSystem() {}

	virtual void tryRelease(Id id) = 0;
	virtual void cleanup() = 0;
};

// ---------------------------------------------------------------------------

template <typename Id, typename T>
class ComponentSystem : public BaseSystem<Id> {
  private:
	using container = std::vector<T>;

	std::size_t const n;
	container data;
	std::vector<std::size_t> lookup;

  protected:
	std::vector<Id> unused;

  public:
	ComponentSystem(std::size_t n=100u);
	virtual ~ComponentSystem();

	/// Create and return an object with the given id
	/**
	 * @pre The id mustn't be used, yet.
	 * @pre The id must be > 0.
	 * @pre The id must be < MAX_OBJECTS.
	 * @post The object's data is initialized according to T's default ctor.
	 * @param id to use
	 * @return reference to the object
	 */
	T& acquire(Id id);

	/// Release the given object
	/**
	 * The object is only marked for deletion. You need to call `cleanup()`
	 * after iteration to remove all marked objects.
	 * @pre The object must exist.
	 * @param id to identify the object with
	 */
	void release(Id id);

	/// Query whether an object with the given id exists or not
	/**
	 * @return true if the described object exists
	 */
	bool has(Id id) const;

	/// Query const reference to an object
	/**
	 * @pre The object must exist.
	 * @param id of the desired object
	 * @return const reference to the object
	 */
	T const& query(Id id) const;

	/// Query non-const reference to an object
	/**
	 * @pre The object must exist.
	 * @param id of the desired object
	 * @return non-const reference to the object
	 */
	T& query(Id id);

	void tryRelease(Id id) override;

	/// Cleanup unused object
	/**
	 * Call this method to cleanup all unused objects.
	 */
	virtual void cleanup() override;

	/// Query number of used components
	std::size_t size() const;

	/// Query maximum number of usable components
	std::size_t capacity() const;

	using iterator = typename container::iterator;
	using const_iterator = typename container::const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};

}  // ::utils

// include implementation details
#include <utils/component_system.inl>
