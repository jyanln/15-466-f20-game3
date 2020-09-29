#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

  struct Object {
    Scene::Transform* self = nullptr;

    glm::vec3 velocity;

    std::shared_ptr<Sound::PlayingSample> sound;
  } asteroid0, asteroid1, asteroid2;

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

  virtual void setup();
  virtual void spawn_object(int type);

  static constexpr float ship_speed = 1.0f;
  static constexpr float asteroid_speed = 10.0f;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, boost, brake;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *miner = nullptr;

  float brightness = 1.0f;

  glm::vec3 miner_pos = glm::vec3(0.f, 0.f, 0.f);

  bool running = true;

  float score = 0;

  float countdown1 = 2.5f;
  float countdown2 = 5.f;

	//camera:
	Scene::Camera *camera = nullptr;
};
