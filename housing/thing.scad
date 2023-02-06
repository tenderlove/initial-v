REAR_POST_Z = 31;
POST_DIAMETER = 6;
POST_OUTER_DIAMETER = 14;
REAR_TAB_LENGTH = 16;
MIN_WALL_THICKNESS = 2;
FRONT_POST_BASE = 15;
TOP_PLATE_Y = 101;
BOX_X = 81;
MOUNTING_HOLE_Z = 13.54;
BOX_Y = TOP_PLATE_Y;
FRONT_WALL_THICKNESS = MIN_WALL_THICKNESS + 1;
BOX_OUTER_Y = BOX_Y + FRONT_WALL_THICKNESS;
BOX_OUTER_X = BOX_X + (2 * MIN_WALL_THICKNESS);
BOX_INNER_Y = BOX_Y;
JACK_X = 9.5;
JACK_Z = 11;
JACK_FROM_RIGHT = 4.2;
BOTTOM_FRONT_POST_Z = 48;
BOTTOM_THICKNESS = MIN_WALL_THICKNESS;

module ShifterThing() {
  translate([-5, 6, -1])
    linear_extrude(4)
    rotate([0, 0, -16])
    rotate([0, 180, 0])
    //import("path413.svg", center=true, dpi=94.5);
    import("handle.svg", center=true, dpi=24.6);
}

module TopPlate(width) {
  difference() {
    color("blue")
    linear_extrude(2)
      square([width, TOP_PLATE_Y], center=true);

    //translate([0, 0, -1])
    //  linear_extrude(4)
    //  square([width - 4, TOP_PLATE_Y - 4], center=true);
    ShifterThing();
  }
}

module RearPost(height) {
  $fn = 80;
  diameter = POST_DIAMETER;

  outer_diameter = POST_OUTER_DIAMETER;

  length = (outer_diameter / 2) + (REAR_TAB_LENGTH - outer_diameter);
  echo(length);

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
  }
}

module FrontPosts(height) {
  translate([0, TOP_PLATE_Y / 2, 0]) {
    translate([0, (TOP_PLATE_Y / 2) - (FRONT_POST_BASE / 2), 0]) {
      //x = (BOX_X / 2) + 7;
      x = (BOX_X / 2) + (POST_OUTER_DIAMETER / 2);
      translate([-x, 0, 0])
        rotate([0, 0, 90])
        FrontPost(height);

      translate([x, 0, 0])
        rotate([0, 0, -90])
        FrontPost(height);
    }
  }
}

module FrontPost(height) {
  $fn = 80;
  diameter = POST_DIAMETER;

  outer_diameter = POST_OUTER_DIAMETER;

  length = (outer_diameter / 2) + (16 - outer_diameter);
  post_outer_radius = POST_OUTER_DIAMETER / 2;

  difference() {
    linear_extrude(height)
      union() {
        circle(d = outer_diameter);
        translate([-(FRONT_POST_BASE / 2), -post_outer_radius])
          polygon([[0, 0], [FRONT_POST_BASE, 0], [14.5, post_outer_radius], [0.5, post_outer_radius]]);
      }

    translate([0, 0, -1])
    linear_extrude(height + 2)
      circle(d = diameter);
  }

}

module RearWall(height, post_height) {
  rear_post_center_x = (59 / 2) + (POST_DIAMETER / 2);

  for (i = [-1, 1]) {
    translate([i * rear_post_center_x, 0, 0])
      RearPost(post_height);
  }

  top_fill_y = REAR_TAB_LENGTH - (POST_OUTER_DIAMETER / 2);
  translate([0, 0, post_height - height]) {
    // Top plate between tabs
    translate([0, -(top_fill_y / 2), height - MIN_WALL_THICKNESS])
      linear_extrude(MIN_WALL_THICKNESS)
      // Just adding +2 to make it slightly wider so everything is connected nicely
      square([((rear_post_center_x * 2) - POST_OUTER_DIAMETER) + 2, top_fill_y], center=true);

    linear_extrude(height)
      square([(rear_post_center_x * 2) - POST_OUTER_DIAMETER, 2], center=true);
  }
}

module TopPart() {
  FrontPosts(REAR_POST_Z + 2); // front are ~2mm lower

  translate([0, TOP_PLATE_Y / 2, 0]) {
    translate([0, 0, 2]) {
      translate([0, -((TOP_PLATE_Y / 2) + 9), 0])
        rotate([0, 0, 180])
        RearWall(REAR_POST_Z, REAR_POST_Z);
      translate([0, 0, REAR_POST_Z - 2])
        TopPlate(BOX_X);
    }
  }

  difference() {
    TopOuterBox(REAR_POST_Z + MIN_WALL_THICKNESS);
    FrontHole();
    LeftWallSlot();
  }
}

module FrontHole() {
  width = 31;
  height = 21;
  center = 22 / 2;
  depth = 5;

  translate([-center, TOP_PLATE_Y - (depth / 2) , 2])
    linear_extrude(height)
    square([width, MIN_WALL_THICKNESS + depth]);
}

module LeftWallSlot() {
  translate([-((BOX_X / 2) + 1.5), 4.5, -1])
    linear_extrude(8)
    square([2, 22]);
}

module TopOuterBox(box_z) {
  x_axis_wall = FRONT_WALL_THICKNESS;
  y_axis_wall = MIN_WALL_THICKNESS + 1;
  box_with_wall_y = BOX_Y + x_axis_wall;
  box_with_wall_x = BOX_X + (2 * y_axis_wall);
  post_overlap = 4;

  difference() {
    translate([-(box_with_wall_x / 2), -post_overlap, 0])
      linear_extrude(box_z)
      square([box_with_wall_x, box_with_wall_y + post_overlap]);

    translate([-(BOX_X / 2), -(1 + post_overlap), -1])
      linear_extrude(MIN_WALL_THICKNESS + box_z + 2)
      square([BOX_X, BOX_Y + 1 + post_overlap]);
  }
}

module SuctionMounts() {
  top_shift = REAR_TAB_LENGTH / 2;

  // Front suction mount
  translate([0, (BOX_Y / 2) + top_shift + FRONT_WALL_THICKNESS, 0])
    rotate([0, 0, 180])
    SuctionMount();

  // Rear suction mount
  translate([0, -((BOX_Y / 2) + (top_shift * 2)), 0])
    SuctionMount();

  // Side mounts
  for (i = [1, -1]) {
    translate([i * ((BOX_OUTER_X / 2) + top_shift), 12, 0])
      rotate([0, 0, i * 90])
      SuctionMount();
  }
}

module BottomTray(wall_height, front_post_height) {
  post_overlap = 4;

  height = wall_height + BOTTOM_THICKNESS;
  post_height = front_post_height + BOTTOM_THICKNESS;
  rear_wall_height = height + 2;
  rear_post_height = post_height + 2;

  translate([0, -(BOX_Y / 2), 0]) {
    difference() {
      translate([-(BOX_OUTER_X / 2), -post_overlap, 0])
        cube([BOX_OUTER_X, BOX_OUTER_Y + post_overlap, height]);

      // Inner cutout
      translate([-(BOX_X / 2), -(1 + post_overlap), BOTTOM_THICKNESS])
        cube([BOX_X, BOX_Y + 1 + post_overlap, BOTTOM_THICKNESS + height]);

      // Front cutout
      translate([-(BOX_X / 2), BOX_Y - 0.5, BOTTOM_THICKNESS])
        cube([16.5, 2, height + 1]);
    }

    // rear wall
    translate([0, -(REAR_TAB_LENGTH - (POST_OUTER_DIAMETER / 2)), rear_post_height])
      rotate([180, 0, 0])
      RearWall(height, rear_post_height);

    FrontPosts(post_height);
  }

  //SuctionMounts();
}

module SuctionMount() {
  top_shift = (POST_OUTER_DIAMETER / 2) + (REAR_TAB_LENGTH - POST_OUTER_DIAMETER);

  translate([0, -top_shift + (REAR_TAB_LENGTH / 2), 0])

  difference() {
    post_outer_radius = POST_OUTER_DIAMETER / 2;
    nut_depth = 4;
    tab_height = nut_depth + MIN_WALL_THICKNESS;
    tab_length = REAR_TAB_LENGTH;
    union() {
      cylinder(tab_height, d=POST_OUTER_DIAMETER, $fn=80);
      translate([-post_outer_radius, 0, 0])
        cube([POST_OUTER_DIAMETER, post_outer_radius + (tab_length - POST_OUTER_DIAMETER), tab_height]);
    }
    translate([0, 0, tab_height - nut_depth])
      cylinder(nut_depth + 0.5, d=9.3, $fn=6);
    translate([0, 0, -0.5])
      cylinder(tab_height + 2, d=5.3, $fn=80);
  }
}

module PowerJackCutout() {
  translate([(-(JACK_X / 2)) + (BOX_X / 2) - JACK_FROM_RIGHT, BOX_OUTER_Y / 2, (JACK_Z / 2) + 6])
    cube([JACK_X, 5, JACK_Z], center = true);
}

module ProgrammingCableCutout() {
  translate([(-(JACK_X / 2)) + (BOX_X / 2) - JACK_FROM_RIGHT - JACK_X - 2, (BOX_OUTER_Y / 2) - 5, -BOTTOM_THICKNESS])
    cube([11, 3, 5], center=true);
}

module BackTabCutout() {
  rear_tab_z = 6;
  rear_tab_x = 14;
  rear_tab_top = 42;
  back_wall = (BOX_Y / 2) + (REAR_TAB_LENGTH - (POST_OUTER_DIAMETER / 2));
  translate([0, 0, rear_tab_top - rear_tab_z])
    translate([0, -back_wall, rear_tab_z / 2])
    cube([rear_tab_x, MIN_WALL_THICKNESS + 2, rear_tab_z], center = true);
}

module PCBMount() {
  mount_width = 3.5;
  mount_height = 11;
  pcb_thickness = 1.8;

  translate([-mount_width, -((MIN_WALL_THICKNESS * 2) + pcb_thickness) / 2, 0]) {
    translate([0, pcb_thickness + MIN_WALL_THICKNESS, 0])
      cube([mount_width, MIN_WALL_THICKNESS, mount_height]);
    cube([mount_width, MIN_WALL_THICKNESS, mount_height]);
  }
}

module BottomPart() {
  difference() {
    translate([0, 0, -BOTTOM_THICKNESS])
      BottomTray(BOTTOM_FRONT_POST_Z + MOUNTING_HOLE_Z, BOTTOM_FRONT_POST_Z);
    PowerJackCutout();
    ProgrammingCableCutout();
    BackTabCutout();
    x = (BOX_X / 2) + ((MIN_WALL_THICKNESS) / 2);
    y = (TOP_PLATE_Y / 2) - (FRONT_POST_BASE / 2);
    for (i = [-1, 1]) {
      translate([i * x, y, 0])
        translate([-(MIN_WALL_THICKNESS + 2) / 2, -FRONT_POST_BASE / 2, 48])
        cube([MIN_WALL_THICKNESS + 2, FRONT_POST_BASE, MOUNTING_HOLE_Z + 1]);
    }
    cutout_y = 22;
    cutout_z = 21;
    cutout_x = 1;
    translate([(-cutout_x) - (BOX_X / 2), (-BOX_Y / 2) + 3, BOTTOM_FRONT_POST_Z])
      cube([cutout_x + 1, cutout_y, cutout_z]);
  }
  translate([BOX_X / 2, (BOX_INNER_Y / 2) - (FRONT_POST_BASE / 2), 0])
    PCBMount();
}

difference() {
BottomPart();
  translate([-100, -30, -5])
    cube([200, 500, 200]);
}

//TopPart();

//RearWall(48, 50);
