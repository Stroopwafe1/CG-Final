#pragma once
#include <vector>
#include "glm/glm.hpp"

/*
Forward declaration of SceneObject to counter-act circular dependency
*/
class SceneObject;

/*
The type of animations that you can apply.
ROTATE will rotate the object
SCALE will scale the object
TRANSLATE will translate the object
MOVETO will translate the object from the current absolute position to the desired absolute position in the precise amount of steps
*/
enum class AnimationType {
	ROTATE, SCALE, TRANSLATE, MOVETO
};

/*
If the animation should repeat
NO_REPEAT will clear the animation when all stages are done
REPEAT will loop the animation from the beginning when all stages are done
REVERSE will go back and forth from the beginning to the end and back to the beginning again and back to the end, etc...
*/
enum class AnimationRepeat {
	NO_REPEAT, REPEAT, REVERSE
};

struct AnimationStage {
	AnimationType Type; // The type of animation
	float AdditionalInput; // Additional input (used for rotation angle)
	glm::vec3 Transformation; // The transformation used for the animation
	int End; // The end number of steps for the stage to end
	AnimationStage(AnimationType type, glm::vec3 transformation, int end, float additionalInput=1)
		: Type(type), AdditionalInput(additionalInput), Transformation(transformation), End(end) {
	}
};

class Animation {
private:
	AnimationRepeat m_Repeating; // The repeating type of this animation
	int m_CurrentStage; // The current stage of this animation
	int m_PrevStage; // The previous stage of this animation (used to invalidate m_TranslationCache)
	std::vector<AnimationStage> m_Stages; // The stages of this animation
	int m_CurrentStep; // The current step in the current stage (when equal to AnimationStage.End, the current stage is done)
	glm::vec3 m_TranslationCache = glm::vec3(-100); // The cache of the translation for AnimationType::MOVETO
	bool m_ReverseRepeatForward; // The current direction for the AnimationRepeat::REVERSE logic

public:
	// Methods documented in Animation.cpp
	Animation(const AnimationRepeat& repeat = AnimationRepeat::REPEAT);
	void AddStage(const AnimationStage& stage);
	void Animate(SceneObject* object);

private:
	void ApplyTransformation(SceneObject* object, const AnimationStage stage);
	void HandleEndOfStage(SceneObject* object, const AnimationStage stage);
};
