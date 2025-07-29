outer_wall_thickness = 1.5;
bolt_height=2;

module inner_bounds(r) {
  intersection() {
    hull() children();
    minkowski() {
      difference() {
        minkowski() {
          children();
          circle(r=0.1, $fn=32);
        }
        children();
      }
      circle(r=r, $fn=32);
    }
  }
}
  
module outer_bounds(r) {
  difference() {
    minkowski() {
      children();
      circle(r=r, $fn=32);
    }
    children();
  }
}

module wall() {
  translate([outer_wall_thickness + 25.37/2, outer_wall_thickness + 46.55/2,0])
    render()
      outer_bounds(outer_wall_thickness)
        import("hub.svg", center=true);
}




module support_hub() {
  total_height = 20;
  debord_height=2.5;

  linear_extrude(total_height)
    wall();



  linear_extrude(height=2)
    inner_bounds(debord_height)
      wall();


  y_max = 16;

  fixation_y = y_max - bolt_height;
  fixation_sx = 8;
  hole_x = 4;
  translate([-fixation_sx, fixation_y, 0]) {
    difference() {
      cube([fixation_sx, bolt_height, total_height]);
      translate([fixation_sx / 2, 0, 10 + debord_height])
        hull()
          for(zdlt=[-2, 2])
            translate([0,0,zdlt])
              rotate([-90,0,0])
                translate([0,0,-1])
                  cylinder(d=3.2, $fn=32, h=bolt_height + 2);
    }
  }
    
}

scale([1,1,-1])
  support_hub();