{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  # nativeBuildInputs is usually what you want -- tools you need to run
  nativeBuildInputs = with pkgs.buildPackages; [
    pkg-config
	  bear
	  glslang # or shaderc
	  vulkan-headers
	  vulkan-loader
    vulkan-validation-layers
	  lua
	  freetype
	  cglm
	  SDL2
	  gdb
    clang-tools
	  # (callPackage ./SDL3.nix {})
	  (callPackage ./json-c.nix {})
	  (callPackage ./vma.nix {})
  ];
}
