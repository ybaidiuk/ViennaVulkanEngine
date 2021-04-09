/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"



namespace ve {

	///simple event listener for rotating objects
	class RotatorListener : public VEEventListener {
		VESceneNode *m_pObject = nullptr;
		float m_speed;
		glm::vec3 m_axis;
	public:
		///Constructor
		RotatorListener(std::string name, VESceneNode *pObject, float speed, glm::vec3 axis) :
			VEEventListener(name), m_pObject(pObject), m_speed(speed), m_axis(glm::normalize(axis)) {};

		void onFrameStarted(veEvent event) {
			glm::mat4 rot = glm::rotate( glm::mat4(1.0f), m_speed*(float)event.dt, m_axis );
			m_pObject->multiplyTransform(rot);
		}

		bool onSceneNodeDeleted(veEvent event) {
			if (m_pObject == event.ptr) return true;
			return false;
		};
	};


	///simple event listener for rotating objects
	class BlinkListener : public VEEventListener {
		VEEntity *m_pEntity;
		double t_now = 0.0;
		double t_last = 0.0;
		double m_blinkDuration;

	public:
		///Constructor
		BlinkListener(std::string name, VEEntity *pEntity, double duration) : 
			VEEventListener(name), m_pEntity(pEntity), m_blinkDuration(duration) {};

		///\brief let cubes blink
		void onFrameStarted(veEvent event) {
			t_now += event.dt;
			double duration = t_now - t_last;

			if (duration > m_blinkDuration) {
				m_pEntity->m_visible = m_pEntity->m_visible ? false : true;	//toggle visibility
				t_last = t_now;
			}
		}

		///\returns true if this event listener instance should be deleted also
		bool onSceneNodeDeleted(veEvent event) {
			if (m_pEntity == event.ptr) return true;
			return false;
		};

	};


	///simple event listener for loading levels
	class LevelListener : public VEEventListener {
	public:
		///Constructor
		LevelListener(std::string name) : VEEventListener(name) {};

		///load a new level when pressing numbers 1-3
		virtual bool onKeyboard(veEvent event) {
			if (event.idata3 == GLFW_RELEASE) return false;

			if (event.idata1 == GLFW_KEY_1 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(1);
				return true;
			}

			if (event.idata1 == GLFW_KEY_2 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(2);
				return true;
			}

			if (event.idata1 == GLFW_KEY_3 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(3);
				return true;
			}
			return false;
		}
	};


	///simple event listener for switching on/off light
	class LightListener : public VEEventListener {
	public:
		///Constructor
		LightListener(std::string name) : VEEventListener(name) {};

		///\brief switch on or off a given light
		void toggleLight(std::string name) {
			VELight *pLight = (VELight*)getSceneManagerPointer()->getSceneNode(name);
			if (pLight == nullptr) return;

			pLight->m_switchedOn = pLight->m_switchedOn ? false : true;
		}

		///load a new level when pressing numbers 1-3
		virtual bool onKeyboard(veEvent event) {
			if (event.idata3 == GLFW_RELEASE) return false;

			if (event.idata1 == GLFW_KEY_8 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardDirLight");
				return true;
			}

			if (event.idata1 == GLFW_KEY_9 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardPointLight");
				return true;
			}

			if (event.idata1 == GLFW_KEY_0 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardSpotLight");
				return true;
			}

			return false;
		}
	};



	///user defined manager class, derived from VEEngine
	class MyVulkanEngine : public VEEngine {
	protected:

	public:
		/**
		* \brief Constructor of my engine
		* \param[in] debug Switch debuggin on or off
		*/
		MyVulkanEngine( bool debug=false) : VEEngine(debug) {};
		~MyVulkanEngine() {};

		///Register an event listener to interact with the user
		virtual void registerEventListeners() {
			VEEngine::registerEventListeners();

			registerEventListener(new LevelListener("LevelListener"), { veEvent::VE_EVENT_KEYBOARD });
		};

        void createPlane(VESceneNode *parent){
            VESceneNode *plane;
            VECHECKPOINTER(plane = getSceneManagerPointer()->loadModel("Plane", "media/models/test/plane", "14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj", aiProcess_FlipUVs | aiProcess_FlipWindingOrder));
            plane->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(5.f, 5.f, 5.f)));
            plane->multiplyTransform(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
            plane->multiplyTransform(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
            plane->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 15.0f)));
            parent->addChild(plane);
        }

		///create many lights
		void createLights(uint32_t n, VESceneNode *parent) {
			float stride = 200.0f;
			static std::default_random_engine e{ 12345 };
			static std::uniform_real_distribution<> d{ 1.0f, stride };

			for (uint32_t i = 0; i < n; i++) {
				VELight *pLight;
				VECHECKPOINTER(pLight = getSceneManagerPointer()->createLight("Light" + std::to_string(i), VELight::VE_LIGHT_TYPE_SPOT, parent));
				pLight->m_col_diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
				pLight->m_col_specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
				pLight->m_param[0] = stride/3.0f;
				//pLight->multiplyTransform(glm::translate(glm::vec3(d(e) - stride / 2.0f, d(e)/10.0f, d(e) - stride / 2.0f)));

				pLight->lookAt(glm::vec3(d(e) - stride / 2.0f, d(e) / 30.0f, d(e) - stride / 2.0f), 
					glm::vec3(d(e) - stride / 2.0f, 0.0f, d(e) - stride / 2.0f),
					glm::vec3( 0.0f, 1.0f, 0.0f ));
			}
		}

		///Load the first level into the game engine
		///The engine uses Y-UP, Left-handed
		virtual void loadLevel( uint32_t numLevel=1) {

			VEEngine::loadLevel(numLevel );			//create standard cameras and lights

			VESceneNode *pScene;
			VECHECKPOINTER( pScene = getSceneManagerPointer()->createSceneNode("Level 1", getRoot()) );
	
			VESceneNode *e4;
			VECHECKPOINTER( e4 = getSceneManagerPointer()->loadModel("The Plane", "media/models/test", "plane_t_n_s.obj",0, pScene) );
			e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));

			VEEntity *pE4;
			VECHECKPOINTER( pE4 = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0") );
			pE4->setParam( glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f) );

			VESceneNode *e1;
			VECHECKPOINTER( e1 = getSceneManagerPointer()->loadModel("The Cube", "media/models/test/crate0", "cube.obj"));
			e1->multiplyTransform( glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)));
			e1->multiplyTransform( glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 5.0f, 10.0f)));
			pScene->addChild(e1);
            createPlane(pScene);
		};
	};
}


using namespace ve;

int main() {

	bool debug = false;
#ifdef  _DEBUG
	debug = true;
#endif

	MyVulkanEngine mve(false);	//enable or disable debugging (=callback, validation layers)

	mve.initEngine();
	mve.loadLevel(1);
	mve.run();

	return 0;
}

