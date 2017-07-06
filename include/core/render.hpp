#pragma once
#include <core/common.hpp>
#include <core/dungeon.hpp>
#include <core/event.hpp>
#include <core/entity.hpp>

#include <utils/layered_sprite.hpp>
#include <utils/lighting_system.hpp>

namespace core {

namespace render_impl {

using Renderables = std::vector<RenderData const*>;

/// Contains all data that has been collected through culling
struct CullingBuffer {
	// basic rendering
	utils::EnumMap<Terrain, sf::VertexArray> terrain;
	utils::EnumMap<ObjectLayer, Renderables> objects;
	// ambiences
	std::vector<sf::Sprite const *> ambiences;
	// lighting
	std::vector<utils::Edge> edges;
	std::vector<utils::Light> lights;
	// player highlighting
	std::vector<sf::Sprite const*> highlights;
	// debugging
	sf::VertexArray grid;

	CullingBuffer();
};

/// helper structure to keep implementation signatures clean and tidy
struct Context {
	LogContext& log;
	RenderManager& render_manager;
	AnimationManager const& animation_manager;
	MovementManager const& movement_manager;
	DungeonSystem& dungeon_system;
	CameraSystem& camera_system;
	utils::LightingSystem& lighting_system;

	mutable std::vector<CullingBuffer> buffers;
	sf::Color grid_color;
	bool cast_shadows;
	mutable sf::Shader sprite_shader;

	Context(LogContext& log, RenderManager& render_manager,
		AnimationManager const& animation_manager,
		MovementManager const& movement_manager, DungeonSystem& dungeon_system,
		CameraSystem& camera_system, utils::LightingSystem& lighting_system);
};

}  // ::render_impl

// ---------------------------------------------------------------------------
// Render System

/// The render system performs object, terrain and lighting rendering
/**
 *	It is the link between different component systems in order to manipulate
 *	each sprite's representation according to the object's world position,
 *	looking direction and animation state. Those data are retrieved directly
 *	from the components by querying their managers. Each affecting information
 *	is masked by a dirtyflag to determine whether is has changed since the
 *	last time the sprite was culled and drawn. Sprites that are never
 *	displayed will never be updated, despite their state and dirtyflags
 *	changed or not.
 *	Furthermore, multiple cameras are supported in order to cull multiple
 *	player views at once. When culling, the corresponding CullingBuffer is
 *	modified. When finally drawing, the lighting system (if enabled) is also
 *	used to draw ranged objects covered by the shadows of the terrain.
 *	Keep in mind to cull once per draw call. A typical frame is started by
 *	updating the system to allow camera movement and zooming. This can be
 *	done multiple times. Finally the system can be drawn by culling and
 *	drawing explicitly.
 *	Changing a sprite texture is triggered via event
 */
class RenderSystem : public sf::Drawable
					 // Event API
					 ,
					 public utils::EventListener<SpriteEvent>
					 // Component API
					 ,
					 public RenderManager {

  protected:
	render_impl::Context context;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
	RenderSystem(LogContext& log, std::size_t max_objects, AnimationManager const& animation_manager,
		MovementManager const& movement_manager, DungeonSystem& dungeon_system,
		CameraSystem& camera_system, utils::LightingSystem& lighting_system);

	void setCastShadows(bool flag);
	void setGridColor(sf::Color color);

	void handle(SpriteEvent const& event);

	void update(sf::Time const& elapsed);
	void cull();
};

namespace render_impl {

// ---------------------------------------------------------------------------
// Internal Render API

/// Determine rotation angle of an object
/**
 *	Returns the rotation angle (in degree) for the given object. Looking south
 *	is used as 0°, other directions imply clock-wise rotations. The object's
 *	looking direction must be one of 8 possible directions: S, SW, W, NW, N,
 *	NE, E, SE.
 *
 *	@pre vector needs to be a valid looking direction
 *	@param data Vector to use
 *	@return rotation angle in degree
 */
float getRotation(sf::Vector2i const& vector);

/// Update the sprite leg's texture
/**
 *	The sprite is specified by the given layer. Its texture will be updated.
 *
 *	@param context RenderContext to work with
 *	@param data RenderData to update for
 *	@param layer SpriteLegLayer that specifies the actual sprite
 *	@param texture Texture pointer to use
 */
void updateTexture(Context& context, RenderData& data, SpriteLegLayer layer,
	sf::Texture const* texture);

/// Update the sprite torso's texture
/**
 *	The sprite is specified by the given layer. Its texture will be updated.
 *
 *	@param context RenderContext to work with
 *	@param data RenderData to update for
 *	@param layer SpriteTorsoLayer that specifies the actual sprite
 *	@param texture Texture pointer to use
 */
void updateTexture(Context& context, RenderData& data, SpriteTorsoLayer layer,
	sf::Texture const* texture);

/// Applies frame data to a sprite
/**
 *	This applies a frame's texture clipping rectangle and frame origin to the
 *	given sprite object.
 *
 *	@param frame Frame data to apply
 *	@param sprite Sprite to apply data to
 */
void applyFrame(utils::Frame const& frame, sf::Sprite& sprite);

/// Applies all animation data to an object
/**
 *	This applies all animation data to the given object. This includes all
 *	layers of frame-based animations as well as brightness- and saturation-
 *	based animations.
 *
 *	@param ani_data AnimationData of the object that should be applied
 *	@param data RenderData of the object that should be updated
 */
void applyAnimation(AnimationData const& ani_data, RenderData& data);

/// Updates an object's renderable state
/**
 *	This updates the given object's renderable state according to its
 *	component's data. This results in a new transformation matrix and/or
 *	updated sprite states (rectangle and origin) per layer. Those data are
 *	only applied if the corresponding components' dirtyflags are set.
 *	Otherwise the object will remain it its previous state.
 *
 *	@pre The object is assigned to a dungeon.
 *	@param context Rendering context to work with
 *	@param data RenderData that should be to updated
 */
void updateObject(Context& context, RenderData& data);

/// Updates all cameras position and zoom
/**
 *	This is used to update all cameras position and zoom since the last update.
 *	A camera's position depends on the position of its objects. Its zoom
 *	depends on the distance between those objects. The larger their distance
 *	the wider the zoom to be able to display all objects.
 *	It will also guarantee that the correct number of culling buffers are
 *	prepared.
 *
 *	@pre Each camera has at least one object.
 *	@pre Each camera's object is assigned to a dungeon.
 *	@post context.buffers.size() == context.camera_system.size()
 *	@param context Rendering context to work with
 *	@param elapsed Elapsed time since last update
 */
void updateCameras(Context& context, sf::Time const& elapsed);

// --------------------------------------------------------------------

// Cull ambiences of the given tile
/// @param buffer CullingBuffer to write to
/// @param cell Dungeon cell to cull from
void cullAmbiences(CullingBuffer& buffer, DungeonCell const & cell);

/// Collect edges from an entity for causing shadows
/// The entity's sprite's rect is used as edge. Also the entity's
/// current position is used.
///
/// @param context Const reference to underlying context
/// @param data Const reference to RenderData
/// @param edges Out parameter for edges
void addEdges(Context const & context, RenderData const & data, std::vector<utils::Edge>& edges);

/// Culls all relevant data for a specific camera to a buffer
/**
 *	This is used to cull all relevant data (terrain tiles, layer-sorted
 *	objects, lighting edges and light souces, as well as debugging information)
 *	to the given buffer. According to the given camera and dungeon, all cells
 *	outside the visible area are not fetched.
 *	While culling, the corresponding objects are updated referring to their
 *	renderable state if necessary.
 *
 *	@param context Rendering context to work with
 *	@param buffer CullingBuffer to write to
 *	@param cam Camera to use for determining the visible area
 *	@param dungeon Dungeon to use for coordinate transformation.
 */
void cullScene(Context& context, CullingBuffer& buffer, CameraData const& cam,
	Dungeon& dungeon);

/// Cull all scenes to their corresponding buffers
/**
 *	This is used to cull all scenes to their corresponding buffers by calling
 *	cullScene() per camera/buffer.
 *
 *	@pre Each camera has at least one object.
 *	@pre Each camera's object is assigned to a dungeon.
 *	@param context Rendering context to work with
 */
void cullScenes(Context& context);

/// Draw all ambience sprites to the render target
/// @param buffer CullingBuffer to use
/// @param target Render target
void drawAmbiences(CullingBuffer const & buffer, sf::RenderTarget& target);

/// Draw all highlighting sprites to the render target
/// @param buffer CullingBuffer to use
/// @param target Render target
void drawHighlightings(CullingBuffer const & buffer, sf::RenderTarget& target);

/// Draw all sprites to the render target
/**
 *	This is used to draw all renderables to the given target. Each renderable
 *	contains of multiple layered sprites. While drawing, the entire rendering
 *	state (including colorization) is used by applying a suitable shader.
 *
 *	@param context Rendering context to work with
 *	@param objects Array of render components to draw
 *	@param target RenderTarget to draw to
 */
void drawSprites(Context const& context, Renderables const& objects,
	sf::RenderTarget& target);

/// Draw the entire scene to the render target
/**
 *	This is used to draw all elements inside the culling buffer to the given
 *	render target. The correct rendering order is obeyed here to achieve a
 *	well-layered, lighting-based scene representation.
 *
 *	@param context Rendering context to work with
 *	@param buffer CullingBuffer to draw
 *	@param target RenderTarget to draw to
 *	@param cam Camera to use for drawing
 *	@param dungeon Dungeon that specifies the tileset texture
 */
void drawScene(Context const& context, CullingBuffer& buffer,
	sf::RenderTarget& target, CameraData const& cam, Dungeon& dungeon);

/// Draw all scenes to the render target
/**
 *	This is used to draw all provided scenes to a single render target. It
 *	will call drawScene() per camera/buffer.
 *
 *	@pre Each camera has at least one object.
 *	@pre Each camera's object is assigned to a dungeon.
 *	@param context Rendering context to work with
 *	@param target RenderTarget to draw to
 */
void drawScenes(Context const& context, sf::RenderTarget& target);

}  // ::render_impl

}  // ::core
