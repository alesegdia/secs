#pragma once

#include <memory>
#include <string>

#define SECS_SYSTEM(name) const char* GetName() const override { return #name; }

namespace secs
{

	/**
	 * @brief The System class is a base class for all systems in the ECS (Entity Component System).
	 */
	class System
	{
	public:

		typedef std::shared_ptr<System> Ptr;

		/**
		 * @brief Virtual destructor for System.
		 */
		virtual ~System();

		/**
		 * @brief Steps the system by the given delta time.
		 * @param delta The delta time.
		 */
		virtual void Step(double delta);

		/**
		 * @brief Renders the system.
		 */
		virtual void RenderStep();

		/**
		 * @brief Called before updating the system.
		 * @param delta The delta time.
		 */
		virtual void PreUpdate(double delta);

		/**
		 * @brief Called after updating the system.
		 * @param delta The delta time.
		 */
		virtual void PostUpdate(double delta);

		/**
		 * @brief Sets the step configuration for the system.
		 * @param process_step Whether the system has a processing step.
		 * @param render_step Whether the system has a rendering step.
		 */
		void SetStepConfiguration(bool process_step, bool render_step);

		/**
		 * @brief Enables the system.
		 */
		void Enable();

		/**
		 * @brief Disables the system.
		 */
		void Disable();

		/**
		 * @brief Checks if the system is enabled.
		 * @return True if the system is enabled, false otherwise.
		 */
		bool IsEnabled();

		/**
		 * @brief Checks if the system has a processing step.
		 * @return True if the system has a processing step, false otherwise.
		 */
		bool HasProcessingStep();

		/**
		 * @brief Checks if the system has a rendering step.
		 * @return True if the system has a rendering step, false otherwise.
		 */
		bool HasRenderingStep();

		/**
		 * @brief Gets the execution priority of the system.
		 * @return The execution priority of the system.
		 */
		int GetExecutionPriority();

		virtual const char* GetName() const { return "<UNNAMED>"; }

	protected:

		/**
		 * @brief Sets the execution priority of the system.
		 * @param priority The execution priority.
		 */
		void SetExecutionPriority(int priority);

	private:
		bool m_hasProcessingStep = true; ///< Indicates if the system has a processing step.
		bool m_hasRenderingStep = false; ///< Indicates if the system has a rendering step.
		bool m_isEnabled = true; ///< Indicates if the system is enabled.
		int m_executionPriority = 0; ///< The execution priority of the system.

	};

}