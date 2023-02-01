BOX_X = 81;
BOX_Y = 101;
BOX_Z = 95;
MIN_WALL_THICKNESS = 2;
SIDE_WALL_THICKNESS = 3;
JACK_X = 9.5;
JACK_Y = 8;
JACK_Z = 11;
JACK_FROM_RIGHT = 4.2;
TAB_LENGTH = 16;
TAB_Z = 14;

POST_DIAMETER = 6;
POST_OUTER_DIAMETER = 14;
FRONT_POST_BASE = POST_OUTER_DIAMETER + 1;

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
  // Front left bar thing
  translate([0, BOX_Y, 0])
    cube([16.5, 2, 51]);
}

module MainBox() {
  cube([BOX_X, BOX_Y, BOX_Z]);
}

module FrontTopJunk() {
  x = 31;
  z = 21;
  center = 22 / 2;
  y = SIDE_WALL_THICKNESS + 3;

  translate([28, BOX_Y, BOX_Z - z - 8])
    cube([x, y, z]);
}

module PowerJack() {
  x_shift = BOX_X - JACK_X - 7;
  translate([x_shift, BOX_Y, 6])
    cube([JACK_X, JACK_Y, JACK_Z]);
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

module FrontPost(height) {
  $fn = 80;
  length = (POST_OUTER_DIAMETER / 2) + (16 - POST_OUTER_DIAMETER);
  post_outer_radius = POST_OUTER_DIAMETER / 2;
  base_difference = (FRONT_POST_BASE - POST_OUTER_DIAMETER) / 2;

  difference() {
    linear_extrude(height)
      union() {
        circle(d = POST_OUTER_DIAMETER);
        translate([-(FRONT_POST_BASE / 2), -post_outer_radius])
          polygon([ [0, 0],
                    [FRONT_POST_BASE, 0],
                    [POST_OUTER_DIAMETER + base_difference, post_outer_radius],
                    [base_difference, post_outer_radius]
                  ]);
      }

    translate([0, 0, -1])
    linear_extrude(height + 2)
      circle(d = POST_DIAMETER);
  }

}

module FrontMounts() {
  x = FRONT_POST_BASE;
  y = TAB_LENGTH;
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
  MainBox();
  FrontLeftBump();
  RearTab();
  FrontTopJunk();
  PowerJack();
  LeftSideBump();
  FrontMounts();
  RearMounts();
  ShifterRing();
}

module RearBox() {
  x = BOX_X - (POST_OUTER_DIAMETER * 2);
  y = TAB_LENGTH - (POST_OUTER_DIAMETER / 2);
  z = BOX_Z;

  translate([(BOX_X / 2) - (x / 2), -y, 0])
  cube([x, y, z]);
}

Shifter();
RearBox();
