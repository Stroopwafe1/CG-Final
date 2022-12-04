#include "Animation.h"
#include "SceneObject.h"

Animation::Animation(const AnimationRepeat& repeat) {
	m_Repeating = repeat;
	m_CurrentStage = 0;
	m_CurrentStep = 0;
	m_PrevStage = 0;
	m_ReverseRepeatForward = false;
}

/*
Adds an animation stage to the vector of stages
@param stage - The animation stage to add
*/
void Animation::AddStage(const AnimationStage& stage) {
	m_Stages.push_back(stage);
}

/*
Handles the animation logic for the provided object.
1. Gets the current stage and resets the translate cache if the last stage ended.
2. Applies the transformation based on the animation type of the current stage
3. Determines what the next stage should be based on the repeat, or clears the animation if AnimationRepeat::NO_REPEAT was specified
@param object - A pointer to the object to animate
*/
void Animation::Animate(SceneObject* object) {
	AnimationStage stage = m_Stages.at(m_CurrentStage);

	// Reset translation cache
	if (m_CurrentStage != m_PrevStage) {
		m_TranslationCache = glm::vec3(-100);
		m_PrevStage = m_CurrentStage;
	}

	// Apply transformation
	ApplyTransformation(object, stage);

	m_CurrentStep++;

	// Check for end of stage, and determine next stage based on repeat
	HandleEndOfStage(object, stage);
}

/*
Applies the transformation specified by the AnimationType of the given stage
@param object - The object to apply the transformation to
@param stage - The current stage
*/
void Animation::ApplyTransformation(SceneObject* object, const AnimationStage stage) {
	switch (stage.Type) {
	case AnimationType::ROTATE:
		object->Rotate(stage.AdditionalInput, stage.Transformation);
		break;
	case AnimationType::SCALE:
		object->Scale(stage.Transformation);
		break;
	case AnimationType::TRANSLATE:
		object->Translate(stage.Transformation);
		break;
	case AnimationType::MOVETO:
		glm::vec3 trans;
		if (m_TranslationCache == glm::vec3(-100)) {
			glm::vec3 diff = stage.Transformation - object->m_Position;
			trans = diff / (float)stage.End;
			m_TranslationCache = trans;
		} else
			trans = m_TranslationCache;
		object->Translate(trans);
		break;
	}
}

/*
Handles the end of the current stage
@param object - The object to clear the animation for if AnimationRepeat::NO_REPEAT was specified
@param stage - The current stage to end
*/
void Animation::HandleEndOfStage(SceneObject* object, const AnimationStage stage) {
	if (m_CurrentStep < stage.End) return;
	m_CurrentStep = 0;
	m_PrevStage = m_CurrentStage;
	switch (m_Repeating) {
	case AnimationRepeat::NO_REPEAT:
		m_CurrentStage++;
		if (m_CurrentStage >= m_Stages.size())
			object->ClearAnimation();
		break;
	case AnimationRepeat::REPEAT:
		m_CurrentStage = (m_CurrentStage + 1) % m_Stages.size();
		break;
	case AnimationRepeat::REVERSE:
		if (m_Stages.size() == 1)
			break;
		if (m_ReverseRepeatForward)
			m_CurrentStage++;
		else
			m_CurrentStage--;

		if (m_CurrentStage < 0) {
			m_CurrentStage = 1;
			m_ReverseRepeatForward = true;
		} else if (m_CurrentStage >= m_Stages.size()) {
			m_CurrentStage = m_Stages.size() - 2;
			m_ReverseRepeatForward = false;
		}
		break;
	}
}