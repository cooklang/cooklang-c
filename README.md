
  

# [WIP] cook-in-c

  

Cooklang parser implemented in C

  

  

## Install:

 
Download the code as a zip file and extract, or use the git clone command to download:

```
git clone https://github.com/cooklang/cook-in-c.git
```

  

  

## Documentation


A recipe can be parsed by using the parseRecipe function:
```C
Recipe  *  parseRecipe(  char  *  fileName  );
```
Parsed recipes come in the form of C structs and are declared as:
```C
typedef  struct {
	List  *  metaData;
	List  *  stepList;
} Recipe;
```

An example execution flow could look something like this:
```C
int  main(  int argc,  char  ** argv ){
	++argv;
	--argc;

	Recipe * parsedRecipe = parseRecipe(argv[0]);

	return 1;
}
```
