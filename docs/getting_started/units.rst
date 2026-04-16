==============================
Units, Scales and Measurements
==============================

Nova Physics is a unit-agnostic physics engine. It does not have any internal dependency on one unit or measurement system. The user decides their own units and how it makes sense in their own applications.

However, while you're free to choose your own unit system, there are things to keep in mind to get the best results from the engine.

Object Scales
=============

Even if Nova Physics doesn't rely on specific units, it still has internal tolerances. Values that control things like how much overlap is allowed before a collision is resolved, how fast errors are corrected and such. These are tuned around a certain scale of numbers. When your bodies are very large or very small relative to that scale, those thresholds stop working correctly and your simulations may become jittery and feel "off".

A common mistake is sizing bodies to match pixels directly, for example creating a 800x600 unit body because the sprite is 800x600 pixels on screen. You should resist this as your rendering scale and your simulation scale are independent concerns. Instead, keep your body sizes in a comfortable range and handle the translation to screen coordinates in your renderer.

This comfortable range can be roughly 0.5-10 units for most games, considering the default penetration slop setting is 0.05 units (*penetration slop* is the amount of penetration error allowed before collisions are corrected). Floating point precision is another thing to keep in mind.

Unit Systems
============

Okay, but, what is a "unit"? As said above, Nova Physics is unit-agnostic. The user is responsible for what they input and what to expect as output from the engine.

Here's a table of physics properties and what they represent if we try to abide by real-world SI units.

We are going to say `1 unit` = `1 meter`. And the simulation is going to be simulated in `Δt` = `1/60 seconds`. (The mass unit is chosen as `kg`, but what's important is the relative masses, not the unit itself.)

+------------------+------------+
| Property         | Unit       |
+==================+============+
| Position         | m          |
+------------------+------------+
| Velocity         | m/s        |
+------------------+------------+
| Acceleration     | m/s²       |
+------------------+------------+
| Density          | kg/m²      |
+------------------+------------+
| Mass             | kg         |
+------------------+------------+
| Inertia          | kg * m²    |
+------------------+------------+
| Force            | kg * m/s²  |
+------------------+------------+
| Torque           | kg * m²/s² |
+------------------+------------+
| Angle            | rad        |
+------------------+------------+
| Angular velocity | rad/s      |
+------------------+------------+
| Penetration slop | m          |
+------------------+------------+