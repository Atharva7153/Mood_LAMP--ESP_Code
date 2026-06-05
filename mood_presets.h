/*
 * ============================================================================
 * FILE: mood_presets.h
 * ============================================================================
 * PURPOSE: Define predefined color presets for different moods/activities
 *          Each mood has RGB values optimized for specific environments
 * 
 * MOOD PRESETS INCLUDED:
 * 1. Focus      - Cool blue tones for concentration and productivity
 * 2. Cozy       - Warm orange/golden tones for relaxation and comfort
 * 3. Social     - Warm peachy tones for social gatherings
 * 4. Golden Hour - Warm sunset colors for evenings
 * 5. Late Night - Deep purple/blue tones for night time
 * 6. Rainy Day  - Cool gray-blue tones for calm, introspective mood
 * 7. Creative Flow - Vibrant purple and cyan for creative work
 * 8. Romantic   - Pink and magenta tones for romantic atmosphere
 * 
 * EACH MOOD HAS:
 *   - singleColor: Primary color for solid color mode
 *   - multi1, multi2, multi3: Three colors for smooth animation
 * 
 * USAGE EXAMPLE:
 *   setSingleColor(focus.singleColor);  // Blue solid light
 *   setMultiColors({focus.multi1, focus.multi2, focus.multi3});  // Blue animation
 * 
 * RGB COLOR SPACE:
 *   Each color is defined as CRGB(Red, Green, Blue)
 *   Each component ranges from 0-255:
 *   - 0 = no color, 255 = full intensity
 *   - Example: CRGB(255, 0, 0) = pure red
 *   - Example: CRGB(255, 231, 186) = warm white
 * ============================================================================
 */

#ifndef MOOD_PRESETS_H
#define MOOD_PRESETS_H

#include <FastLED.h>

/*
 * STRUCT: Mood
 * ==========================================
 * PURPOSE: Group color presets for a specific mood
 * 
 * MEMBERS:
 *   - singleColor: Main color for solid mode (primary mood representation)
 *   - multi1: First color in animation sequence
 *   - multi2: Second color in animation sequence
 *   - multi3: Third color in animation sequence
 * 
 * WHY TWO MODES?
 *   - Some users prefer solid light (simple, stable)
 *   - Others prefer animated light (more engaging, dynamic)
 *   - Single = more subtle, Multi = more dynamic experience
 * ==========================================
 */
struct Mood {
  CRGB singleColor;   // Solid color mode

  CRGB multi1;        // Animation frame 1
  CRGB multi2;        // Animation frame 2
  CRGB multi3;        // Animation frame 3
};

// ============================================================================
// MOOD 1: FOCUS
// ============================================================================
// BEST FOR: Studying, work, concentration, productivity
// LIGHTING CHARACTERISTIC: Cool, alert, enhances mental clarity
// COLOR PSYCHOLOGY:
//   - Blue: Calms mind, improves focus
//   - Cool tones: Reduces fatigue, maintains alertness
// AI/COLOR NOTE:
//   - Blue light stimulates wakefulness (avoid before sleep)
//   - Best for daytime productivity tasks
Mood focus = {
  CRGB(220, 235, 255),  // Cool white-blue (light, bright, focused)

  CRGB(220, 235, 255),  // Cool white-blue (start)
  CRGB(180, 240, 255),  // Light cyan (mid)
  CRGB(140, 190, 255)   // Sky blue (end)
};

// ============================================================================
// MOOD 2: COZY
// ============================================================================
// BEST FOR: Relaxing, unwinding, comfort, warmth
// LIGHTING CHARACTERISTIC: Warm, inviting, intimate
// COLOR PSYCHOLOGY:
//   - Orange: Warmth, comfort, relaxation
//   - Amber tones: Mimics firelight, very soothing
// AI/COLOR NOTE:
//   - Warm colors trigger relaxation response
//   - Low correlated color temperature (warm) = natural evening light
Mood cozy = {
  CRGB(255, 170, 90),   // Warm orange (inviting, warm)

  CRGB(255, 170, 90),   // Warm orange (start)
  CRGB(255, 130, 70),   // Deep orange (mid)
  CRGB(255, 210, 150)   // Light orange (end)
};

// ============================================================================
// MOOD 3: SOCIAL
// ============================================================================
// BEST FOR: Parties, gatherings, social events, entertaining
// LIGHTING CHARACTERISTIC: Warm, energetic, welcoming
// COLOR PSYCHOLOGY:
//   - Warm peachy tones: Flattering to skin tones
//   - Welcoming, social atmosphere
// AI/COLOR NOTE:
//   - Peachy-orange most flattering for faces
//   - Warm light makes spaces feel larger, more open
Mood social = {
  CRGB(255, 170, 140),  // Warm peach (flattering, social)

  CRGB(255, 170, 140),  // Warm peach (start)
  CRGB(255, 120, 120),  // Coral-pink (mid)
  CRGB(255, 210, 100)   // Warm peachy (end)
};

// ============================================================================
// MOOD 4: GOLDEN HOUR
// ============================================================================
// BEST FOR: Evenings, sunset ambiance, golden hour photography, nostalgia
// LIGHTING CHARACTERISTIC: Rich warm amber, dramatic sunset feel
// COLOR PSYCHOLOGY:
//   - Golden/amber: Warmth, comfort, nostalgia
//   - Mimics natural sunset lighting (triggers happiness)
// AI/COLOR NOTE:
//   - Replicates magic hour lighting for visual warmth
//   - Very pleasing to the eye, natural-looking
Mood goldenHour = {
  CRGB(255, 140, 70),   // Rich golden (warm, dramatic)

  CRGB(255, 220, 120),  // Light golden (start - bright)
  CRGB(255, 150, 70),   // Medium golden (mid)
  CRGB(180, 90, 40)     // Deep golden (end - sunset tone)
};

// ============================================================================
// MOOD 5: LATE NIGHT
// ============================================================================
// BEST FOR: Night time, sleeping prep, reducing eye strain, calm bedtime
// LIGHTING CHARACTERISTIC: Deep, low color temperature, sleep-friendly
// COLOR PSYCHOLOGY:
//   - Deep purple: Calming, sleep-inducing
//   - Low blue content: Won't suppress melatonin
// AI/COLOR NOTE:
//   - Red/purple have minimal melatonin suppression
//   - Ideal for evening/bedtime to maintain sleep schedule
//   - Reduces blue light exposure before sleep
Mood lateNight = {
  CRGB(70, 60, 160),    // Deep purple (calming, sleep-inducing)

  CRGB(70, 60, 160),    // Deep purple (start)
  CRGB(30, 40, 120),    // Dark blue (mid)
  CRGB(120, 80, 200)    // Purple (end)
};

// ============================================================================
// MOOD 6: RAINY DAY
// ============================================================================
// BEST FOR: Gray days, introspection, calm focus, concentration
// LIGHTING CHARACTERISTIC: Cool gray-blue, atmospheric, contemplative
// COLOR PSYCHOLOGY:
//   - Gray-blue: Calm, meditative, introspective
//   - Cool tones: Subtle, not overstimulating
// AI/COLOR NOTE:
//   - Replicates natural overcast sky lighting
//   - Promotes calm, focused thinking
//   - Soft cool tones reduce visual fatigue
Mood rainyDay = {
  CRGB(140, 160, 180),  // Soft gray-blue (atmospheric)

  CRGB(100, 130, 180),  // Cool gray-blue (start)
  CRGB(160, 220, 220),  // Cyan (mid)
  CRGB(180, 180, 200)   // Lavender-gray (end)
};

// ============================================================================
// MOOD 7: CREATIVE FLOW
// ============================================================================
// BEST FOR: Creative work, design, brainstorming, artistic activities
// LIGHTING CHARACTERISTIC: Vibrant, dynamic, inspiring, energetic
// COLOR PSYCHOLOGY:
//   - Purple: Creativity, imagination, artistic expression
//   - Cyan: Energy, freshness, innovation
//   - Mix: Stimulating and inspiring
// AI/COLOR NOTE:
//   - Vibrant colors stimulate creative thinking
//   - Purple + Cyan combination highly energizing
//   - Good for brainstorming and creative problem-solving
Mood creativeFlow = {
  CRGB(180, 80, 255),   // Vibrant purple (creative, energetic)

  CRGB(180, 80, 255),   // Vibrant purple (start)
  CRGB(80, 220, 255),   // Bright cyan (mid)
  CRGB(255, 80, 180)    // Magenta-pink (end)
};

// ============================================================================
// MOOD 8: ROMANTIC
// ============================================================================
// BEST FOR: Romantic settings, intimate moments, date nights, special occasions
// LIGHTING CHARACTERISTIC: Soft pink/magenta, intimate, passionate
// COLOR PSYCHOLOGY:
//   - Pink: Love, romance, tenderness
//   - Magenta: Passion, mystery, romance
//   - Soft tones: Intimate and comfortable
// AI/COLOR NOTE:
//   - Pink lighting universally flattering and romantic
//   - Magenta adds passion and energy
//   - Soft, dim pink creates intimate ambiance
Mood romantic = {
  CRGB(255, 120, 180),  // Soft pink (romantic, intimate)

  CRGB(255, 120, 180),  // Soft pink (start)
  CRGB(255, 80, 100),   // Rose pink (mid)
  CRGB(220, 150, 255)   // Magenta (end)
};

#endif