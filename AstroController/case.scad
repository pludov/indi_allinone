module cube_rounded(sze, ray) {
  function dir(d) = (d == 0 ? 1 : -1);
  hull() {
    for(X = [0, 1])
      for(Y = [0, 1])
        translate([X * sze[0] + dir(X) * ray, Y * sze[1] + dir(Y) * ray, 0])
          cylinder(r = ray, h=sze[2]);
    }
}
rj45_w=15.3;
rj45_l=15.8;
rj45_h=15;
rj45_l_butee=3.2;
rj45_h_butee=3.9;

bme_y = 19.10;
bme_z = 26.10;
bme_facade_x = 2;
bme_x = 1.7 + bme_facade_x;



// Connector for RJxxx adaptation
// It is separated to keep it printable.
rj_window_y = 38;
rj_window_z = 19;


hole_heater = 11.7 + 0.2;
// Diam jack DC
hole_dc = 7.8 + 0.2;

pcb_sze = [ 110, 76 ];
pcb_size = [ 110, 76];
pcb_ref = [ 1.16, -0.40 ];
inch = 25.4;
pcb_holes =
 [ [ (3.1 - pcb_ref[0]) * inch, pcb_size[1] - (-0.1 - pcb_ref[1]) * inch],
   [ (5.2 - pcb_ref[0]) * inch, pcb_size[1] - (-0.1 - pcb_ref[1]) * inch],
   [ (5.3 - pcb_ref[0]) * inch, pcb_size[1] - ( 2.3 - pcb_ref[1]) * inch],
   [ (1.4 - pcb_ref[0]) * inch, pcb_size[1] - ( 2.3 - pcb_ref[1]) * inch]
 ];

outer_z = 32;


wall_y = 1.5;
// Matches the size of the USB hub (115)
wall_x0 = 1.5;
// Wide enough to allow fixation for hub
wall_x1 = 6;
wall_z = 1.5;

inner_sze = [ pcb_sze[0] + 2, pcb_sze[1] + 2, outer_z -  2 * wall_z];

heater_connections = [
  [16 + 18 * 0,outer_z *0.66],
  [16 + 18 * 1,outer_z *0.66],
  [16 + 18 * 2,outer_z *0.66],
];

dc_connections_y0 = [
  [16 + 18 * 1.5, outer_z * 0.33]
];


module from_root_to_case() {
  children();
}

module from_case_to_bme() {
  translate([-4.2,58,0])  
    children();
}


module bme_plus() {
  from_case_to_bme() {
    translate([-4,-3,0])
      cube_rounded([bme_x + 6, bme_y + 6, outer_z - wall_z], wall_x0, $fn=16);
    
    translate([-3.1,bme_y / 2, outer_z - wall_z -bme_z / 2])
    scale([0.6,(bme_y + 4)/bme_z,1])
      difference() {
        
          sphere(r = bme_z / 2);
        
          sphere(r = bme_z / 2 - 1);
        
        for(J = [ -2, 0, 2])
          rotate([0,J * 15 ,0]) {
            for(I = [ -60,-30, 0, 30, 60 ]) {
              rotate([0,0, I])
                rotate([0,-90,0]) cylinder(d=3.6, $fn=4, h=15);
              rotate([0,I, 0])
                rotate([0,-90,0]) cylinder(d=3.6, $fn=4, h=15);
            }
          }
        for(J = [-3,-1, 1, 3])
          rotate([0,J * 15 ,0])
          for(I = [ -45, -15,15, 45 ])
            rotate([0,0, I])
              rotate([0,-90,0]) cylinder(d=3.6, $fn=4, h=15);
      }
  }
}

module bme_minus() {
  from_case_to_bme() {
    translate([0,0,outer_z - wall_z - bme_z]) {
      cube([bme_x, bme_y, bme_z + 0.1]);
      translate([-3.1,1,0])
        cube([10, bme_y -2, bme_z + 0.1]);
    }
    translate([-3.1,bme_y / 2, outer_z - wall_z -bme_z / 2])
      rotate([0,-90,0])
        translate([0,0,-0.1])
          scale([1,(bme_y + 4)/bme_z,1])
            cylinder(r =bme_z /2 - 1.5, h = 1.2, $fn=32);
  }
}

module bme_facade() {

  hole_dst = 21;
  hole_diam = 2.9;
  hole_y = 3.90 - 2.9 / 2;
  
  module opening() {
    translate([bme_z / 2, 6.8, 0])
      cube([3.3, 3.3, bme_facade_x *2.2], center = true);

    translate([0,0,bme_facade_x - 1.2 + 0.3])
      translate([bme_z / 2, 6.8, 0])
        for(Z=[0:0.1:bme_facade_x - 1.2 + 5]) {
          translate([0,0, -1 -Z ])
            cube([2 * Z + 3.3, 2 * Z + 3.3, 2], center=true);
        }
    // Petit rabot pour que ça passe...
    translate([-1,-1,-1])
      cube([bme_z + 2, bme_y + 2, 1.2]);
  }
  
  
  difference() {
    union() {
      cube_rounded([bme_z, bme_y, bme_facade_x], 1, $fn=16);
    }
    translate([1,1,bme_facade_x - 1.2 ])
      cube_rounded([bme_z -2, bme_y -2, bme_facade_x], 1, $fn=16);
    opening();
  }
  difference() {
    for(I=[-1,1]) {
      translate([bme_z / 2 + I * hole_dst / 2, hole_y, 0]) {
        cylinder(d=hole_diam - 0.2, h= bme_x - 0.3, $fn= 64);
        cylinder(d=hole_diam + 1.5, h= bme_facade_x, $fn= 64);
      }
    }
    opening();
  }

  difference() {
    translate([bme_z / 2, 6.8, 0]) {
      rotate([0,0,45])
      cylinder(r=3.7,$fn=4, h = bme_facade_x);
    }
    opening();
  }
  
  
}

!bme_facade();


// Rotate so x+ gets inside the box
module from_case_to_rj_window() {
   // Translate to the x1 wall
  translate([inner_sze[0] + 2 * wall_y, 8 + rj_window_y, outer_z - wall_z - rj_window_z])
    rotate([0,0,180])
    children();
}

module from_inner_to_pcb() {
  translate([1,1,0])
    children();
}

module from_case_to_inner() {
  translate([wall_y, wall_x0, wall_z])
    children();
}

// Floor
module low_part() {

  from_root_to_case() {
    rounding = 1.5;
    difference () {

      cube_rounded(sze = [inner_sze[0] + 2 * wall_y, inner_sze[1] + wall_x0 + wall_x1, outer_z - wall_z], ray=rounding, $fn=32);
      from_case_to_inner() {
        cube_rounded(sze = inner_sze + [0,0,10], ray = rounding, $fn=32);
      }
    }
  }
}

difference() {
  union() {
    low_part();
    bme_plus();
  }
  bme_minus();
}



color("green") 
  from_root_to_case()
    from_case_to_inner()
      from_inner_to_pcb()
        translate([0,0,1.1])
          cube([pcb_sze[0], pcb_sze[1], 1.9]);
 
from_root_to_case()
  from_case_to_inner()
    from_inner_to_pcb()
      for(hole = pcb_holes)
        translate(hole) 
          cylinder(d=5.8,h=1,$fn=16);

color("red")
from_root_to_case() {
  for(heater = heater_connections) {
    translate([heater[0], -1, heater[1]])
      rotate([-90,0,0])
        cylinder(d=hole_heater, $fn=64, h = 6);
    
  }
  for(dc = dc_connections_y0)
    translate([dc[0], -1, dc[1]])
      rotate([-90,0,0])
        cylinder(d=hole_dc, $fn=64, h = 6);
  from_case_to_rj_window() {
    translate([-0.1,0,0])
      cube([5,rj_window_y, rj_window_z + 0.05]);
  }
}



