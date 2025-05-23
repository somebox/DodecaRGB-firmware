#include "wandering_particles_scene.h"
#include "PixelTheater/SceneKit.h" // shorter aliases
#include "benchmark.h"   // Include benchmark helpers if used

// Include standard libraries used by the implementations
#include <vector>
#include <memory>
#include <cmath>      // For sqrt
#include <algorithm>  // For std::min, std::max
#include <string>     // For std::to_string in status()

namespace Scenes {

// --- WanderingParticlesScene Implementation ---

void WanderingParticlesScene::setup() {
    set_name("Wandering Particles");
    set_description("Particles that wander across the model surface, affected by gravity.");
    set_version("2.1"); // Update version
    set_author("PixelTheater Team");
    
    // Define parameter ranges and defaults
    const int MIN_PARTICLES = 5, MAX_PARTICLES = 200;  
    const int MIN_FADE = 1, MAX_FADE = 50;
    const float MIN_BLEND = 10.0f, MAX_BLEND = 200.0f;
    const float MIN_GRAVITY = -2.5f, MAX_GRAVITY = 2.5f; // Gravity range
        
    // Define parameters
    param("num_particles", "count", MIN_PARTICLES, MAX_PARTICLES, DEFAULT_NUM_PARTICLES, "clamp", "Number of particles");
    param("fade_amount", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
    param("blend_amount", "range", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Blend intensity");
    param("gravity", "range", MIN_GRAVITY, MAX_GRAVITY, DEFAULT_GRAVITY, "clamp", "Z-axis gravity (+down/-up)"); // Add gravity param
    
    // Estimate model radius (affects particle x,y,z calculations)
    // estimateSphereRadius(); // Removed call
    
    // Initialize particles based on parameters
    initParticles();
    
    logInfo("WanderingParticlesScene setup complete");
}

/* // Removed estimateSphereRadius function body
*/

void WanderingParticlesScene::initParticles() {
    int num_particles = settings["num_particles"];
    logInfo("Creating %d particles...", num_particles);
    particles.clear(); // Clear existing particles
    particles.reserve(num_particles); // Reserve space
    for (int i = 0; i < num_particles; i++) {
        // Create particles using unique_ptr
        particles.push_back(std::make_unique<Particle>(*this, i));
    }
    logInfo("%d Particles created.", (int)particles.size());
}

void WanderingParticlesScene::tick() {
    Scene::tick(); // Call base class tick
    
    // Get current parameter values
    uint8_t fade_amount = static_cast<uint8_t>(settings["fade_amount"]);
    float blend_amount = settings["blend_amount"];
    // int reset_chance = settings["reset_chance"]; // Remove usage
    // Convert reset_chance (1-20) to a probability check (approx 0-255)
    // A higher reset_chance param value means MORE resets
    // int reset_check_threshold = settings["reset_chance"] * (255 / MAX_RESET); // Remove usage

    // Fade all LEDs first
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        // Using the global fade utility function
        fadeToBlackBy(leds[i], fade_amount);
    }
        
    // Update and draw each particle
    for (auto& particle_ptr : particles) {
        if (!particle_ptr) continue; // Skip if unique_ptr is null for some reason
        Particle& particle = *particle_ptr; // Dereference for easier access

        particle.tick(); // Update particle state (position, LED, age, state)
        
        // --- Calculate brightness multiplier based on state ---
        // (This now controls the OVERALL brightness during initial fade-in and final fade-out)
        float brightness_multiplier = 1.0f;
        if (particle.state == ParticleState::FADING_IN) {
            float t = std::clamp(static_cast<float>(particle.age) / static_cast<float>(Particle::FADE_IN_DURATION), 0.0f, 1.0f);
            brightness_multiplier = outQuadF(t); // Use unqualified via SceneKit
        } else if (particle.state == ParticleState::FADING_OUT) {
            float t = std::clamp(static_cast<float>(particle.lifespan - particle.age) / static_cast<float>(Particle::FADE_OUT_DURATION), 0.0f, 1.0f);
            brightness_multiplier = 1.0f - inQuadF(t); // Use unqualified via SceneKit
        }
        // --- End brightness calculation ---
        
        // --- Draw Particle Head (Cross-Fade) ---
        float eased_progress = inOutSineF(particle.transition_progress); // Use unqualified
        float current_led_brightness = (1.0f - eased_progress) * brightness_multiplier;
        float target_led_brightness = eased_progress * brightness_multiplier;

        // Draw current LED (fading out)
        int current_led_idx = particle.current_led_number;
        if (current_led_idx >= 0 && current_led_idx < (int)count) {
            uint8_t blend_amount = static_cast<uint8_t>(current_led_brightness * 255.0f);
            if (blend_amount > 0) {
                nblend(leds[current_led_idx], particle.color, blend_amount);
            }
        }
        
        // Draw target LED (fading in)
        int target_led_idx = particle.target_led_number;
        if (target_led_idx >= 0 && target_led_idx < (int)count) {
             uint8_t blend_amount = static_cast<uint8_t>(target_led_brightness * 255.0f);
             if (blend_amount > 0) {
                nblend(leds[target_led_idx], particle.color, blend_amount);
             }
        }
        // --- End Particle Head Drawing ---
        
        // Draw particle trail (Based on discrete path history)
        for (size_t i = 0; i < particle.path.size(); i++) { // Start from i=0 (current_led) for trail fade relative to head
            int trail_led_idx = particle.path[i];
            if (trail_led_idx < 0 || trail_led_idx >= (int)count) continue; // Skip invalid indices
            
            // Skip drawing the current/target head LEDs again in the trail loop
            if (trail_led_idx == current_led_idx || trail_led_idx == target_led_idx) continue; 
            
            // --- Trail Fade Logic (Power Function) ---
            float base_brightness = 255.0f; 
            float exponent = 2.0f; 
            // Fade based on discrete steps 'i' behind the head in the path history
            float fade_factor = 1.0f / powf(static_cast<float>(i) + 1.0f, exponent); // +1 because i=0 is head
            uint8_t trail_blend = static_cast<uint8_t>(std::clamp(base_brightness * fade_factor, 1.0f, 255.0f));
            // --- End Trail Fade Logic ---
            
            // Apply overall particle brightness (fade in/out) to trail
            uint8_t final_trail_blend = static_cast<uint8_t>(trail_blend * brightness_multiplier);
            
            if (final_trail_blend > 0) { 
                nblend(leds[trail_led_idx], particle.color, final_trail_blend);
            }
        }
    }
    
    // --- ADD PARTICLE INTERACTION LOGIC HERE --- 
    // Check for collisions after all particles have been updated and drawn for this tick
    for (size_t i = 0; i < particles.size(); ++i) {
        if (!particles[i] || particles[i]->current_led_number < 0) continue; // Use current_led_number
        for (size_t j = i + 1; j < particles.size(); ++j) {
            if (!particles[j] || particles[j]->current_led_number < 0) continue; // Use current_led_number
            
            // Check if particles are on the same LED (or potentially transitioning to the same target?)
            // Simplest check: are they both primarily occupying the same LED?
            if (particles[i]->current_led_number == particles[j]->current_led_number) { // Use current_led_number
                // Collision! Randomize angular velocities (direction tendencies) for both
                float max_rand_av = 0.02f; 
                float max_rand_cv = 0.02f;
                
                particles[i]->av = randomFloat(-max_rand_av, max_rand_av);
                particles[i]->cv = randomFloat(-max_rand_cv, max_rand_cv);
                
                particles[j]->av = randomFloat(-max_rand_av, max_rand_av);
                particles[j]->cv = randomFloat(-max_rand_cv, max_rand_cv);
                
                // Optional: Reset their age/hold timer to force immediate movement next tick?
                // particles[i]->age = particles[i]->hold_time; 
                // particles[j]->age = particles[j]->hold_time; 
            }
        }
    }
    // --- END PARTICLE INTERACTION LOGIC --- 

}

// Implementation for the status string    
std::string WanderingParticlesScene::status() const {
    std::string output;
    // Safely access settings - maybe they haven't been initialized?
    int num_particles = particles.size(); // Use actual size
    int fade_amount = 0; 
    float blend_amount = 0.0f;
    // int reset_chance = 0; // Removed
    if (has_parameter("fade_amount")) fade_amount = settings["fade_amount"];
    if (has_parameter("blend_amount")) blend_amount = settings["blend_amount"];
    // if (has_parameter("reset_chance")) reset_chance = settings["reset_chance"]; // Removed
    
    output += "Particles: " + std::to_string(num_particles) + 
              " (fade=" + std::to_string(fade_amount) + 
              ", blend=" + std::to_string(static_cast<int>(blend_amount)) + // Cast float for display
              // ", reset=" + std::to_string(reset_chance) + ")\n"; // Removed reset part
              ")\n";
              
    // Show info for first 3 particles if they exist
    for (size_t i = 0; i < std::min(size_t(3), particles.size()); i++) {
        const auto& p = particles[i];
        if (p) { // Check if unique_ptr is valid
            output += "P" + std::to_string(p->particle_id) + 
                      ": age=" + std::to_string(p->age) + 
                      " led=" + std::to_string(p->current_led_number) + // Use current_led_number
                      " tgt=" + std::to_string(p->target_led_number) + // Show target
                      " prg=" + std::to_string(static_cast<int>(p->transition_progress * 100)) + "%\n"; // Show progress
        }
    }
    return output;
}

} // namespace Scenes 