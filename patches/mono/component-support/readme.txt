the patch mono-component-support.diff contains the guts of support for mono components ( and extensions )

problems
  * some flakyness
     * need to set up custom MONO_PATH & LD_LIBRARY_PATH for openoffice
        * need to create some custom links for some libs
     * some strange random crashes
     * rearranging some of the implementation code around and things don't work  anymore e.g. some mono embed stuff just fails, the same code in its own function works :-0 ( probably I have done something hugely and obviously wrong ) In anycase the above points more than likely point to some memory (mis)management issue. Luckily most of the unmanaged -> managed embedding foo ( besides the already existing bridge code ) is small and in located now in cli_ure/source/mono_loader/mono_loader.cxx
     * Currently there is one ( more or less static ) MonoLoader, this is a c++ class that holds a reference to the real implementation which implemented as a mangaged object ( via the mono embed framework ) 
        * effectively all components are deployed as single assemblies, each assembly that wants to be a component has to implement some certain uno interfaces, more than one component can exist in a single assembly (dll). The 'component' is loaded by reflection. There may be problems if the component/assembly has a reference to another assembly ( that is not one located already by the MonoLoader ) there are situations I guess where it may not be found :-(. If true would be a rather large disadvantage with the approach just described. However though, I have deployed a Calc addin ( see examples/CalcAddin ) which does deliver a dependant side by side assembly ( for a custom uno type deployed as part of the extension ) and I'm glad to say it seems to work fine ( so probably there are just some limitations on what you can do ) IIRC there is a similar situation with java. 

what needs doing
* the code is ugly, I have no c# knowledge ( and I have forgotten most of my java ). it needs at least *some* love ( probably not that much really )
* there are some evil hacks around decoding URLs ( bound to cause problems )
* the loader currently doesn't capture all native ( c# ) exceptions, sometimes these can escape ( without being rethrown to uno Exceptions ) and this can cause some nasty cores ( cleaning the code should solve this )
* the loader is currently only for mono, on windows though we don't want to alienate c# people so a managed c++ loader needs to be written for DotNet ( should be small ). Initially we probably should try to load the DotNet loader on windows and fail back to the Mono loader ( ideally some configuration expressing a preference would be good for that )
* some of the naming reflects the fact I was only thinking about mono and not the wider DotNet/C# world ( e.g. the loader to register c# components is called "org.openoffice.loader.MonoLoader" and the expected media type in the extension bundle is "manifest:media-type="application/vnd.sun.star.uno-component;type=Mono" ) the naming shouldn't be mono specific as you shouldn't care whether your component get loaded by DotNet or Mono
