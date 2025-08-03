module cube_rounded(sze, ray) {
  function dir(d) = (d == 0 ? 1 : -1);
  hull() {
    for(X = [0, 1])
      for(Y = [0, 1])
        translate([X * sze[0] + dir(X) * ray, Y * sze[1] + dir(Y) * ray, 0])
          cylinder(r = ray, h=sze[2], $fn=32);
    }
}

hub_size = [53.5, 19.4];
hub_rounding = 4.5;
// Variables from case.scad
fixation_hub_larg = 2.2;
// De quoi avoir eventuellement une visse de serrage de l'autre coté.
fixation_hub_long = 10;

fixation_height = 7;


languette_size = [fixation_hub_long - 0.2, 10, fixation_hub_larg - 0.2];

module goto_languette() {
    translate([hub_size[0] / 2, hub_size[1] + 1 , -1]) children();
}

languette_screws = [ 5.5, 7.0];
module support() {
    difference() {
        union() {
            // L'exterieur
            translate([-1,-1,-1]) {
                cube_rounded([hub_size[0] + 2, hub_size[1] + 2, fixation_height + 1], hub_rounding);
            }


            // La languette de fixation
            color("red")
            goto_languette() {
                translate([- languette_size[0] / 2, 0,0]) {
                    r = 2;
                    translate([0,-r,0])
                        cube_rounded(languette_size + [0, r, 0], r);
                }
            }
        }

        // Le hub
        cube_rounded([hub_size[0], hub_size[1], fixation_height + 10], hub_rounding);

        // L'accès à sa façade
        clearance = [1.5,1.5,0];
        translate(clearance + [0,0,-1.1])
            cube_rounded([hub_size[0], hub_size[1], fixation_height + 10] - 2 * clearance, hub_rounding);

        goto_languette() {
            hull()
            for(screw = languette_screws){
                translate([0, screw, -1])
                    cylinder(d = 3.2, h = languette_size[2] + 2, $fn=32);
            }
        }

    }

}

module support_ecrou(z_shift) {
    base_w = fixation_hub_long + 2;
    base_z = 18;
    difference() {
        hull() {
            translate([0, -(languette_screws[1] - 3), 0])
                translate([-base_w / 2, 0, 0])
                    cube([base_w, .1, base_z]);

            // Passage pour la visse
            cylinder(d=7.5, h = base_z, $fn=32);
        }

        // Passage pour la visse
        translate([0,0,-0.1])
        cylinder(d=3.1, h = base_z + 0.2, $fn=32);
    }
}


support();

translate([hub_size[0] / 2, - 28, 0])
//goto_languette() {
//    translate([0,languette_screws[1], 2.5])
        for(z_shift = [[0, 0], [0.2, 10], [-0.2, 20]])
            for(X = [ - 15, 0, 15])
                translate([X, z_shift[1], 0])
                    support_ecrou(z_shift[0]);
//}

translate([hub_size[0], -36, 0])
rotate([0, 0, 180])
    support();
