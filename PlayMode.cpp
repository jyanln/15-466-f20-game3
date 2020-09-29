#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES

#include <cmath>
#include <random>

GLuint space_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > space_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("space.pnct"));
	space_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > space_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("space.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = space_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = space_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

#define _USE_MATH_DEFINES

#include <cmath>
Load< Sound::Sample > asteroid_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("asteroid.opus"));
});

PlayMode::PlayMode() : scene(*space_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "craft_miner") miner = &transform;
		else if (transform.name == "Icosphere0") asteroid0.self = &transform;
		else if (transform.name == "Icosphere1") asteroid1.self = &transform;
		else if (transform.name == "Icosphere2") asteroid2.self = &transform;
	}
	if (miner == nullptr) throw std::runtime_error("Miner not found.");
	if (asteroid0.self == nullptr) throw std::runtime_error("Asteroid 0 not found.");
	if (asteroid1.self == nullptr) throw std::runtime_error("Asteroid 1 not found.");
	if (asteroid2.self == nullptr) throw std::runtime_error("Asteroid 2 not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

  setup();
}

PlayMode::~PlayMode() {
}

void PlayMode::setup() {
  brightness = 1.0f;
  spawn_object(0);
  asteroid1.self->position = glm::vec3(1000.f, 0.f, 1000.f);
  asteroid2.self->position = glm::vec3(1000.f, 0.f, 1000.f);

  // Set constant camera and ship orientation
  miner->position = glm::vec3(0.f, 0.f, 0.f);
  camera->transform->position = glm::vec3(0.f, 20.f, 3.f);

  // Set default camera and ship rotation
  miner->rotation = glm::quat(sqrt(2) / 2, sqrt(2) / 2, 0.0f, 0.0f);
  camera->transform->rotation = glm::quat(sqrt(2) / 2, sqrt(2) / 2, 0.0f, 0.0f) * glm::angleAxis(float(M_PI), glm::vec3(0.f, 1.f, 0.f)) * glm::angleAxis(-float(M_PI) * 0.05f, glm::vec3(1.f, 0.f, 0.f));

  score = 0;
  running = true;
}

void PlayMode::spawn_object(int type) {
  static std::mt19937 mt;

  if(type == 0) {
    asteroid0.self->position = glm::vec3(0.f, -60.f, 0.f);
    float dest_x = (mt() / float(mt.max())) * 14.f - 7.f;
    float dest_z = (mt() / float(mt.max())) * 7.f - 3.5f;

    asteroid0.velocity = glm::normalize(glm::vec3(dest_x, 0.f, dest_z) - asteroid0.self->position) * asteroid_speed;

    asteroid0.sound = Sound::loop_3D(*asteroid_sample, 1.0f, asteroid0.self->position, 20.f);
  } else if(type == 1) {
    asteroid1.self->position = glm::vec3(0.f, -60.f, 0.f);
    float dest_x = (mt() / float(mt.max())) * 14.f - 7.f;
    float dest_z = (mt() / float(mt.max())) * 7.f - 3.5f;

    asteroid1.velocity = glm::normalize(glm::vec3(dest_x, 0.f, dest_z) - asteroid1.self->position) * asteroid_speed;

    asteroid1.sound = Sound::loop_3D(*asteroid_sample, 1.0f, asteroid1.self->position, 20.f);
  } else if(type == 2) {
    asteroid2.self->position = glm::vec3(0.f, -60.f, 0.f);
    float dest_x = (mt() / float(mt.max())) * 14.f - 7.f;
    float dest_z = (mt() / float(mt.max())) * 7.f - 3.5f;

    asteroid2.velocity = glm::normalize(glm::vec3(dest_x, 0.f, dest_z) - asteroid2.self->position) * asteroid_speed;

    asteroid2.sound = Sound::loop_3D(*asteroid_sample, 1.0f, asteroid2.self->position, 20.f);
  }
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
      if(!running) {
        setup();
      }
      return true;
    }
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
  if(!running) return;

  score += elapsed;

  if(countdown1 > 0) {
    countdown1 -= elapsed;
    if(countdown1 <= 0) {
      spawn_object(1);
    }
  }
  if(countdown2 > 0) {
    countdown2 -= elapsed;
    if(countdown2 <= 0) {
      spawn_object(2);
    }
  }

  brightness *= 0.998f;
  if(brightness < 0.1) brightness = 0;

  float miner_x = miner->position.x;
  float miner_z = miner->position.z;

  if(up.pressed && !down.pressed) {
    miner_z += ship_speed * elapsed;
  } else if(!up.pressed && down.pressed) {
    miner_z -= ship_speed * elapsed;
  }

  if(left.pressed && !right.pressed) {
    miner_x += ship_speed * elapsed;
  } else if(!left.pressed && right.pressed) {
    miner_x -= ship_speed * elapsed;
  }

  if(miner_x > 7.0f) miner_x = 7.0f;
  else if(miner_x < -7.0f) miner_x = -7.0f;
  if(miner_z > 3.5f) miner_z = 3.5f;
  else if(miner_z < -3.5f) miner_z = -3.5f;

  miner->position = glm::vec3(miner_x, 0.f, miner_z);

  asteroid0.self->position += elapsed * asteroid0.velocity;
  if(asteroid0.self->position.y > 20.f) spawn_object(0);
  asteroid1.self->position += elapsed * asteroid1.velocity;
  if(asteroid1.self->position.y > 20.f) spawn_object(1);
  asteroid2.self->position += elapsed * asteroid2.velocity;
  if(asteroid2.self->position.y > 20.f) spawn_object(2);

  if(glm::distance(miner->position, asteroid0.self->position) < 1.5f ||
  glm::distance(miner->position, asteroid1.self->position) < 1.5f ||
  glm::distance(miner->position, asteroid2.self->position) < 1.5f) {
    running = false;
  }

	{ //update listener to ship position:
		glm::mat4x3 frame = miner->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	boost.downs = 0;
	brake.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f * brightness, 1.0f * brightness, 0.95f * brightness)));
	glUseProgram(0);

	glClearColor(0.5f * brightness, 0.5f * brightness, 0.5f * brightness, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
