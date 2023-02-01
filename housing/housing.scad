BOX_X = 81;
BOX_Y = 101;
BOX_Z = 95;
MIN_WALL_THICKNESS = 2;
WALL = 3;
JACK_X = 9.5;
JACK_Y = 8;
JACK_Z = 11;
JACK_FROM_RIGHT = 4.2;
TAB_LENGTH = 16;
TAB_Z = 14;

POST_DIAMETER = 6;
POST_OUTER_DIAMETER = 14;
FRONT_TAB_Y = POST_OUTER_DIAMETER;
FRONT_TAB_X = POST_OUTER_DIAMETER + 1;

REAR_BOX_Y = TAB_LENGTH - (POST_OUTER_DIAMETER / 2);

// NEGATIVE SPACE

module ShifterRing() {
  translate([BOX_X / 2, BOX_Y / 2, BOX_Z])
  translate([-5, 6, -1])
    linear_extrude(4)
    rotate([0, 0, -16])
    rotate([0, 180, 0])
    //import("path413.svg", center=true, dpi=94.5);
    import("handle.svg", center=true, dpi=24.6);
}

module RearTab() {
  rear_tab_z = 6;
  rear_tab_y = 19;
  rear_tab_x = 14;
  rear_tab_top = 42;
  translate([(BOX_X / 2) - (rear_tab_x / 2), -rear_tab_y, rear_tab_top - rear_tab_z])
    cube([rear_tab_x, rear_tab_y, rear_tab_z]);
}

module FrontLeftBump() {
  x = 16.5;
  y = 2;
  z = 51;
  // Front left bar thing
  translate([0, BOX_Y - 0.1, 0])
    cube([x, y + 0.1, z]);
}

module MainBox() {
  cube([BOX_X, BOX_Y, BOX_Z]);
}

module FrontTopJunk() {
  x = 31;
  z = 21;
  center = 22 / 2;
  y = WALL + 3;

  translate([28, BOX_Y, BOX_Z - z - 8])
    cube([x, y, z]);
}

module PowerJack() {
  x_shift = BOX_X - JACK_X - 7;
  z_shift = 6;
  translate([x_shift, BOX_Y - 0.1, z_shift])
    cube([JACK_X, JACK_Y + 0.1, JACK_Z]);
}

module LeftSideBump() {
  x = 1.6;
  y = 22;
  z = 21;
  z_shift = 48;
  y_shift = 3.6;
  translate([-x, y_shift, z_shift])
  cube([x, y, z]);
}

module FrontMounts() {
  x = FRONT_TAB_X;
  y = FRONT_TAB_Y;
  z = TAB_Z;
  z_shift = 63.3;

  translate([BOX_X / 2, (BOX_Y / 2) - (x / 2), z_shift - z])
    for (i = [0, 1]) {
      mirror([i, 0, 0])
        translate([(y / 2) + (BOX_X / 2), BOX_Y / 2, 0])
        rotate([0, 0, 90])
        translate([0, 0, z / 2])
        cube([x, y, z], center=true);
    }
}

module RearMounts() {
  x = POST_OUTER_DIAMETER;
  y = TAB_LENGTH;
  z = TAB_Z;

  z_shift = 65.3;

  for(i = [0, BOX_X - x]) {
    translate([i, -y, z_shift - z])
      cube([x, y, z]);
  }
}

module Shifter() {
  #MainBox();
  FrontLeftBump();
  RearTab();
  FrontTopJunk();
  PowerJack();
  LeftSideBump();
  FrontMounts();
  RearMounts();
  ShifterRing();
  RearBox();
}

module RearBox() {
  x = BOX_X - (POST_OUTER_DIAMETER * 2);
  y = REAR_BOX_Y;
  z = BOX_Z;

  translate([(BOX_X / 2) - (x / 2), -y, 0])
  cube([x, y + 0.1, z]);
}

// POSITIVE SPACE

module FrontPost(height) {
  $fn = 80;
  post_outer_radius = POST_OUTER_DIAMETER / 2;
  base_difference = (FRONT_TAB_X - POST_OUTER_DIAMETER) / 2;
  y_offset = FRONT_TAB_Y - POST_OUTER_DIAMETER;

  translate([0, -FRONT_TAB_Y / 2, 0])
  difference() {
    linear_extrude(height)
      union() {
        translate([0, y_offset + POST_OUTER_DIAMETER / 2, 0])
          circle(d = POST_OUTER_DIAMETER);

        translate([-(FRONT_TAB_X / 2), 0, 0])
          polygon([ [0, 0],
                     [FRONT_TAB_X, 0],
                     [POST_OUTER_DIAMETER + base_difference, post_outer_radius + y_offset],
                     [base_difference, post_outer_radius + y_offset]
                  ]);
      }

    translate([0, y_offset + (POST_OUTER_DIAMETER / 2), -1])
    linear_extrude(height + 2)
      circle(d = POST_DIAMETER);
  }

}


module OuterBox() {
  translate([-WALL, -(REAR_BOX_Y + WALL), -WALL])
    cube([BOX_X + (WALL * 2), BOX_Y + REAR_BOX_Y + (WALL * 2), JACK_Z + 6 + WALL - 0.1]);
}

module FrontPosts(height) {
  translate([BOX_X / 2, BOX_Y - FRONT_TAB_X, -WALL])
  for (i = [0, 1]) {
    mirror([i, 0, 0])
      translate([-BOX_X / 2, 0, 0])
      rotate([0, 0, 90])
      translate([FRONT_TAB_X / 2, FRONT_TAB_Y / 2, 0])
      FrontPost(height);
  }
}

Shifter();
OuterBox();
FrontPosts(100);
