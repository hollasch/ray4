Change Log -- `ray4-c`
====================================================================================================

# v2.0.0 (in progress)
  - Forked from the original C sources, updated occasionally. This will now be the executable from
    the C sources, with possible future patches, but basically dead-ended. The new version will be
    C++, with some updates.

# v0.1.50 (1992-01-25)
  - Release to public.

# v0.1.42 (1991-05-27)
  - Changed scaline buffer to use a minimum-sized buffer.
  - Implemented the attribute list.
  - Add the -g option.
  - zPlane automatically sets to the first image plane if unspecified or specified at zero.
  - Add hypersphere debug code.
  - Add refraction.
  - Fix logic flaw in HitSphere() t1/t2 selection.
  - Fixed bug in tetrahedron/parallelepiped normal and intersection assignment code; the previous
    version didn't ensure that the passed parameter pointers were non-nil.
  - Fix bug where objects behind point light sources could produce shadows.
  - Fixed bug in image plane seek. The previous version assumed that the image data always began at
    image plane zero (image.first[2]==0).

# v0.1.30 1990-12-17
  - Add parallelepiped.
  - Add the -p option.
  - Add internal intersection verification code for tetrahedra.
  - Added reflections.
  - Fix bug with ray-grid basis-vector generation.
  - Fix bug in GetColorPalette, where not all pixels were read during the first pass, and the
    scanline buffer was not used up completely.
  - Fix ray-triangle intersection.

# v0.1.12 (1990-11-15)
  - Add tetrahedron and triangle.
  - Add IFF output.
  - Add colormap load/save.
  - Add tiled display.

# v0.0.1 (1990-09-30)
  - Initial version.
