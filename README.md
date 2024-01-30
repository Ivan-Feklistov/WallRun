# WallRun
Small fun project with implemented movement mechanics inspired by Titanfall 2's wall running, wall jumping and wall climbing

Here you can see how it looks and works:
https://youtu.be/Y4lbQVUzCfs

Rules:
- When Player touches the wall and is in the air, then they stick to the wall and start to slide along it unless movement buttons are pressed.
- When movement along the wall is happening, Player slides along.
- It doesn't matter where Player look, but if movement direction is away from the wall then wall running stops and Player falls down.
- Also wall run stops if Player is on the same wall for too long (timer).
- No movement backwards allowed on the wall.
- Player can jump from the wall and keep momentum.
- Player can grab the ledge if they look at it and presse forward.

The trick is in changing AirControl and GravityScale (in Movement component) based on state Player is in (in the air, on the wall, on the floor, etc).
Plus some animation visuals (lean weapon to the side, landing head shake).

