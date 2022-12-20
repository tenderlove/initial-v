module ShifterThing() {
  translate([-8, 3, -1])
    linear_extrude(4)
    rotate([0, 0, -17])
    rotate([0, 180, 0])
    import("path413.svg", center=true, dpi=94.5);
}

TOP_PLATE_Y = 101;

module Foo(width) {
  difference() {
    linear_extrude(2)
      square([width, TOP_PLATE_Y], center=true);

    translate([0, 0, -1])
      linear_extrude(4)
      square([width - 4, TOP_PLATE_Y - 4], center=true);
    //ShifterThing();
  }
}

POST_DIAMETER = 6;
POST_OUTER_DIAMETER = 14;
REAR_TAB_LENGTH = 16;
MIN_WALL_THICKNESS = 2;

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

BOX_X = 79;

module FrontPosts(height) {
  x = (BOX_X / 2) + 7;
  translate([-x, 0, 0])
    rotate([0, 0, 90])
    FrontPost(height);

  translate([x, 0, 0])
    rotate([0, 0, -90])
    FrontPost(height);
}

module FrontPost(height) {
  $fn = 80;
  diameter = POST_DIAMETER;

  outer_diameter = POST_OUTER_DIAMETER;

  length = (outer_diameter / 2) + (16 - outer_diameter);

  difference() {
    linear_extrude(height)
      union() {
        circle(d = outer_diameter);
        translate([-(15 / 2), -7])
          polygon([[0, 0], [15, 0], [14.5, 7], [0.5, 7]]);
      }

    translate([0, 0, -1])
    linear_extrude(height + 2)
      circle(d = diameter);
  }

}

module RearWall(height) {
  rear_post_center_x = (59 / 2) + (POST_DIAMETER / 2);

  for (i = [-1, 1]) {
    translate([i * rear_post_center_x, 0, 0])
      RearPost(height);
  }

  top_fill_y = REAR_TAB_LENGTH - (POST_OUTER_DIAMETER / 2);
  // Top plate between tabs
  translate([0, -(top_fill_y / 2), height - MIN_WALL_THICKNESS])
    linear_extrude(MIN_WALL_THICKNESS)
    // Just adding +2 to make it slightly wider so everything is connected nicely
    square([((rear_post_center_x * 2) - POST_OUTER_DIAMETER) + 2, top_fill_y], center=true);

  linear_extrude(height)
    square([(rear_post_center_x * 2) - POST_DIAMETER, 2], center=true);
}

REAR_POST_Z = 31;

translate([0, (TOP_PLATE_Y / 2) - (15 / 2), 0])
  FrontPosts(REAR_POST_Z + 2); // front are ~2mm lower

translate([0, 0, 2]) {
translate([0, -((TOP_PLATE_Y / 2) + 9), 0])
rotate([0, 0, 180])
  RearWall(REAR_POST_Z);
translate([0, 0, REAR_POST_Z - 2])
  Foo(79);
}
