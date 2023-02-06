BOX_X = 81;
BOX_Y = 101;
BOX_Z = 95;
TOP_WALL = 2;
BOTTOM_WALL = 2;
WALL = 3;
JACK_X = 9.5;
JACK_Y = 8;
JACK_Z = 11;
JACK_Z_SHIFT = 8;
JACK_FROM_RIGHT = 6;
TAB_LENGTH = 16;
TAB_Z = 14;

POST_DIAMETER = 6;
POST_OUTER_DIAMETER = 14.5;
FRONT_TAB_Y = POST_OUTER_DIAMETER;
FRONT_TAB_X = POST_OUTER_DIAMETER + 1;
FRONT_TAB_Z_SHIFT = 63.3;

REAR_TAB_X = POST_OUTER_DIAMETER;
REAR_TAB_Y = 16;
REAR_TAB_Z_SHIFT = 65.3;

REAR_SIDE_TO_SIDE = 79;

REAR_BOX_Y = TAB_LENGTH - (POST_OUTER_DIAMETER / 2) - WALL;
REAR_BOX_X = REAR_SIDE_TO_SIDE - (REAR_TAB_X * 2);
assert(REAR_BOX_X == 50);

REAR_TAB_LOOP_Z = 7;
REAR_TAB_LOOP_TOP = 43;

SUCTION_X = 14;
SUCTION_Y = SUCTION_X + 2;

// NEGATIVE SPACE

module ShifterRing() {
  translate([BOX_X / 2, BOX_Y / 2, BOX_Z])
  translate([-5, 6, -1])
    linear_extrude(5)
    rotate([0, 0, -16])
    rotate([0, 180, 0])
    //import("path413.svg", center=true, dpi=94.5);
    import("handle.svg", center=true, dpi=24.6);
}

module RearTabLoop() {
  rear_tab_z = REAR_TAB_LOOP_Z;
  rear_tab_y = 19;
  rear_tab_x = 14;
  rear_tab_top = REAR_TAB_LOOP_TOP;

  x_shift = REAR_TAB_X + (REAR_BOX_X / 2) - (rear_tab_x / 2);
  translate([x_shift, -rear_tab_y, rear_tab_top - rear_tab_z])
    cube([rear_tab_x, rear_tab_y, rear_tab_z]);
}

module FrontLeftBump() {
  x = 17.5;
  y = 1.5;
  z = 55;
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
  y = WALL + 3;

  translate([28, BOX_Y - 0.1, BOX_Z - z - 8])
    cube([x, y + 0.1, z]);
}

module PowerJack() {
  x_shift = BOX_X - JACK_X - JACK_FROM_RIGHT;
  z_shift = JACK_Z_SHIFT;
  translate([x_shift, BOX_Y - 0.1, z_shift])
    cube([JACK_X, JACK_Y + 0.1, JACK_Z]);
}

module Buttons() {
  x = 7.5;
  y = JACK_Y; // Doesn't matter
  z = 19;
  x_shift = BOX_X - x - JACK_FROM_RIGHT - JACK_X - 12.3;
  z_shift = JACK_Z_SHIFT; // same as jack
  translate([x_shift, BOX_Y - 0.1, z_shift])
  cube([x, y + 0.1, z]);
}

module ProgAccess() {
  x = 11;
  y = 3;
  z = BOTTOM_WALL + 2;
  y_shift = BOX_Y - (FRONT_TAB_X / 2);
  translate([BOX_X - x - JACK_X - JACK_FROM_RIGHT, y_shift, -z + 1])
    cube([x, y, z]);
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

// I thought that the tabs were on the edges of the box, but actually the
// bottom right junk is wider.  The tabs are aligned on the left _side_
// of the box, but not the right side.  The right side tabs are a fixed
// distance from the left.
FRONT_TIP_TO_TIP = 107.3;

module FrontMounts() {
  x = FRONT_TAB_X;
  y = FRONT_TAB_Y;
  z = TAB_Z;
  z_shift = FRONT_TAB_Z_SHIFT;

  x_shift = FRONT_TIP_TO_TIP - (FRONT_TAB_Y * 2);

  jitter = 4;

  translate([-(y + (jitter / 2)), BOX_Y - x, z_shift - z])
    cube([y + jitter, x, z]);

  translate([x_shift - (jitter / 2), BOX_Y - x, z_shift - z])
    cube([y + jitter, x, z]);
}

module RearMounts() {
  x = POST_OUTER_DIAMETER;
  y = TAB_LENGTH;
  z = TAB_Z;

  z_shift = REAR_TAB_Z_SHIFT;
  x_shift = REAR_SIDE_TO_SIDE - x;

  jitter = 4; // for fixing rendering on x/z plane

  for(i = [0, x_shift]) {
    translate([i, -(y + (jitter / 2)), z_shift - z])
      cube([x, y + jitter, z]);
  }
}

module Shifter() {
  MainBox();
  FrontLeftBump();
  RearTabLoop();
  FrontTopJunk();
  PowerJack();
  LeftSideBump();
  FrontMounts();
  RearMounts();
  ShifterRing();
  RearBox();
  ProgAccess();
  Buttons();
}

module RearBox() {
  x = REAR_BOX_X;
  y = REAR_BOX_Y;
  z = BOX_Z;

  translate([REAR_TAB_X, -y, 0])
    cube([x, y + 0.1, z]);

  // Jitter to connect rear tabs
  translate([REAR_TAB_X, -y, REAR_TAB_Z_SHIFT - TAB_Z])
    cube([x + 2, y + 0.1, TAB_Z]);
}

// POSITIVE SPACE

module FrontPost(height, positive) {
  $fn = 80;
  post_outer_radius = POST_OUTER_DIAMETER / 2;
  base_difference = (FRONT_TAB_X - POST_OUTER_DIAMETER) / 2;
  y_offset = FRONT_TAB_Y - POST_OUTER_DIAMETER;

  translate([0, -FRONT_TAB_Y / 2, 0])
    if (positive) {
      linear_extrude(height) {
        translate([0, y_offset + POST_OUTER_DIAMETER / 2, 0])
          circle(d = POST_OUTER_DIAMETER);

        translate([-(FRONT_TAB_X / 2), 0, 0])
          polygon([ [0, 0],
              [FRONT_TAB_X, 0],
              [POST_OUTER_DIAMETER + base_difference, post_outer_radius + y_offset],
              [base_difference, post_outer_radius + y_offset]
          ]);
      }
    } else {
      translate([0, y_offset + (POST_OUTER_DIAMETER / 2), -1]) {
        linear_extrude(height + 2)
          circle(d = POST_DIAMETER);
        CounterSink(9.3, 4 + 1, POST_DIAMETER);
      }
    }
}

module InnerBox(height) {
  translate([-WALL, -WALL, 0])
  cube([BOX_X + (WALL * 2), BOX_Y + (WALL * 2), height]);
}

module RearBoxContainer(height) {
  x = REAR_BOX_X;
  y = REAR_BOX_Y + WALL;
  z = height;

  translate([REAR_TAB_X, -y, 0])
  cube([x, y + 0.1, z]);
}

module OuterBox(height) {
  translate([-WALL, -(REAR_BOX_Y + WALL), 0])
    cube([BOX_X + (WALL * 2), BOX_Y + REAR_BOX_Y + (WALL * 2), height]);
}

module FrontPosts(height, positive) {
  x_shift = FRONT_TIP_TO_TIP - (FRONT_TAB_Y * 2);

  translate([0, BOX_Y - FRONT_TAB_X, 0]) {
    rotate([0, 0, 90])
      translate([FRONT_TAB_X / 2, FRONT_TAB_Y / 2, 0])
      FrontPost(height, positive);

    translate([x_shift, 0, 0])
      mirror([1, 0, 0])
      rotate([0, 0, 90])
      translate([FRONT_TAB_X / 2, FRONT_TAB_Y / 2, 0])
      FrontPost(height, positive);
  }
}

LAYER_HEIGHT = 0.2;

module CounterSink(countersink_d, countersink_z, bolt_d, facets = 6, floating = true) {
  cylinder(countersink_z, d=countersink_d, $fn=facets);

  if (floating) {
    // First bridge
    translate([0, 0, countersink_z + (LAYER_HEIGHT / 2)])
      intersection() {
        translate([0, 0, -(LAYER_HEIGHT / 2)])
          cylinder(LAYER_HEIGHT, d=countersink_d, $fn=facets);
        color("green")
          cube([bolt_d, countersink_d, LAYER_HEIGHT], center=true);
      }

    color("blue")
    // Second bridge
    translate([0, 0, countersink_z + (LAYER_HEIGHT / 2) + LAYER_HEIGHT])
      cube([bolt_d, bolt_d, LAYER_HEIGHT], center=true);
  }
}

module RearPost(height) {
  $fn = 80;
  diameter = POST_DIAMETER;

  outer_diameter = POST_OUTER_DIAMETER;
  outer_radius = outer_diameter / 2;

  length = (outer_diameter / 2) + (REAR_TAB_Y - outer_diameter);

  translate([0, (-outer_radius) + (REAR_TAB_Y / 2), 0])
  difference() {
    linear_extrude(height)
      union() {
        circle(d = outer_diameter);
        translate([-(outer_diameter / 2), -length])
          square([outer_diameter, length]);
      }

    translate([0, 0, -1])
    linear_extrude(height + 2)
      circle(d = diameter);

    translate([0, 0, -0.5])
      CounterSink(9.3, 4 + 0.5, diameter);
  }
}

module RearPosts(height) {
  for (i = [0, REAR_BOX_X + REAR_TAB_X]) {
    translate([(REAR_TAB_X / 2) + i, -REAR_TAB_Y / 2, 0])
      rotate([0, 0, 180])
      RearPost(height);
  }
}

module Container() {
  height = BOX_Z + TOP_WALL + BOTTOM_WALL;
  translate([0, 0, -BOTTOM_WALL]) {
    difference() {
      union() {
        InnerBox(height);
        FrontPosts(height, true);
      }
      FrontPosts(height, false);
    }
    RearPosts(height);
    RearBoxContainer(height);
    SuctionMounts();
  }
}

module PCBMount() {
  mount_width = 5.5;
  mount_height = 13;
  pcb_thickness = 1.8;
  wall = 2;

  translate([BOX_X, BOX_Y - (FRONT_TAB_X / 2), 0])
  translate([-mount_width, -((wall * 2) + pcb_thickness) / 2, 0]) {
    translate([0, pcb_thickness + wall, 0])
      cube([mount_width, wall, mount_height]);
    cube([mount_width, wall, mount_height]);
  }
}

module SuctionMount() {
  nut_depth = 4;
  suction_z = nut_depth + BOTTOM_WALL;

  top_shift = (SUCTION_X / 2) + (SUCTION_Y - SUCTION_X);

  translate([0, -top_shift + (SUCTION_Y / 2), 0])

  difference() {
    post_outer_radius = SUCTION_X / 2;
    union() {
      cylinder(suction_z, d=SUCTION_X, $fn=80);
      translate([-post_outer_radius, 0, 0])
        cube([SUCTION_X, top_shift, suction_z]);
    }
    translate([0, 0, suction_z - nut_depth])
      cylinder(nut_depth + 0.5, d=9.3, $fn=6);
    translate([0, 0, -0.5])
      cylinder(suction_z + 2, d=5.3, $fn=80);
  }
}

module SuctionMounts() {
  translate([BOX_X / 2, BOX_Y / 2, 0]) {
    translate([0, -REAR_BOX_Y / 2, 0])
      for (i = [0, 1]) {
        mirror([0, i, 0])
          translate([0, -(BOX_Y + REAR_BOX_Y) / 2, 0])
          translate([0, -SUCTION_Y / 2, 0])
          SuctionMount();
      }

    for (i = [0, 1]) {
      mirror([i, 0, 0])
        translate([(BOX_X / 2) + (SUCTION_Y / 2), 0, 0])
        rotate([0, 0, 90])
        SuctionMount();
    }
  }
}

module FullContainer() {
  difference() {
    Container();
    Shifter();
  }
  PCBMount();
}

module Mask() {
  mask_x = BOX_X + (FRONT_TAB_Y * 2) + 4;
  mask_y = BOX_Y + REAR_BOX_Y + (SUCTION_Y * 2) + 4;
  mask_z = BOX_Z + TOP_WALL + BOTTOM_WALL;
  translate([-FRONT_TAB_Y - 2, -(REAR_BOX_Y + SUCTION_Y + 2), -BOTTOM_WALL])
    cube([mask_x, mask_y, mask_z]);
}

module TopPartMask() {
  x = BOX_X + ((WALL + SUCTION_Y) * 2);
  y = BOX_Y + REAR_BOX_Y + ((WALL + SUCTION_Y) * 2);
  translate([-SUCTION_Y, -REAR_BOX_Y - SUCTION_Y - WALL, -BOTTOM_WALL - 1])
    cube([x, y, REAR_TAB_LOOP_TOP + BOTTOM_WALL - REAR_TAB_LOOP_Z + 1]);

  y2 = BOX_Y + (WALL * 2) + SUCTION_Y;
  translate([-SUCTION_Y, -WALL, -BOTTOM_WALL - 1])
  cube([x, y2, FRONT_TAB_Z_SHIFT + BOTTOM_WALL + 1]);

  RearPosts(REAR_TAB_Z_SHIFT);
}

module TopPart() {
  difference() {
    FullContainer();
    TopPartMask();
  }
}

module BottomTray() {
  difference() {
    FullContainer();
    translate([0, 0, JACK_Z + 6 + BOTTOM_WALL + 1.9])
    Mask();
  }
}

module FrontPartMask() {
  front_mask_z = FRONT_TAB_Z_SHIFT - JACK_Z_SHIFT - JACK_Z;

  // Mask the front
  translate([-WALL, BOX_Y - 1, JACK_Z_SHIFT + JACK_Z])
    cube([BOX_X + (WALL * 2), WALL + 2, front_mask_z]);

  // Mask the posts
  translate([0, 0, JACK_Z_SHIFT + JACK_Z])
  FrontPosts(front_mask_z, true);
}

module BottomPartMask() {
  // Mask the top
  x = BOX_X + ((WALL + SUCTION_Y) * 2);
  y = BOX_Y + REAR_BOX_Y + ((WALL + SUCTION_Y) * 2);
  translate([-SUCTION_Y, -SUCTION_Y, FRONT_TAB_Z_SHIFT])
    cube([x, y, 50]);

  // Mask the back
  back_mask_z = REAR_TAB_LOOP_Z + 30;
  translate([REAR_TAB_X, -REAR_BOX_Y - WALL - 1, REAR_TAB_LOOP_TOP - REAR_TAB_LOOP_Z])
    cube([REAR_BOX_X, REAR_BOX_Y + WALL + 2, back_mask_z]);

  FrontPartMask();
}

rendering = "full";

if (rendering == "top") {
  //rotate([0, 180, 0])
    TopPart();
}

if (rendering == "bottom") {
  difference() {
    FullContainer();
    BottomPartMask();
  }
}

if (rendering == "middle") {
  intersection() {
    FullContainer();
    FrontPartMask();
  }
}

if (rendering == "full") {
  FullContainer();
}
