from distutils.core import setup, Extension
setup(name="cooklangC", version="1.0",
      ext_modules=[Extension("cooklangC", ["src/CooklangExtension.c", "src/CooklangParser.c", "Cooklang.tab.c", "src/LinkedListLib.c", "src/CooklangRecipe.c", "src/ShoppingListParser.c"])])