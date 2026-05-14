// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FlopAI : ModuleRules
{
    public FlopAI(ReadOnlyTargetRules Target) : base(Target)
    {
        // Distribution builds ship without Private/ source -- use precompiled binaries
        if (!System.IO.Directory.Exists(System.IO.Path.Combine(ModuleDirectory, "Private")))
        {
            bUsePrecompiled = true;
            return;
        }

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Require C++20 for concepts/requires clauses used in ResponseBuilder.h and SystemRetrieval.h
        CppStandard = CppStandardVersion.Cpp20;

        
        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                // UMGEditor ships K2Node_CreateWidget.h + K2Node_GeneratedBoundEvent.h
                // under Private/Nodes/. The composer's CompletionNodeHandlers uses
                // __has_include("K2Node_CreateWidget.h") to detect these â€” UBT does
                // NOT recursively search PrivateIncludePaths, so the Private/Nodes
                // subdirectory must be listed explicitly. Without these paths the
                // create_widget / widget_bound_event handlers silently skip
                // registration and apply_spec returns "no handler registered for
                // node type 'create_widget'" at apply time.
                System.IO.Path.Combine(EngineDirectory, "Source", "Editor", "UMGEditor", "Private"),
                System.IO.Path.Combine(EngineDirectory, "Source", "Editor", "UMGEditor", "Private", "Nodes"),
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "SlateCore",
                "InputCore",
                "ApplicationCore",
                "Slate"
				// ... add other public dependencies that you statically link with here ...
			}
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "EditorSubsystem",
                "WebSockets",
                "SSL",
                "Json",
                "JsonUtilities",
                "UnrealEd",
                "LevelEditor",
                "PythonScriptPlugin",
                "HTTP",
                "DeveloperSettings",
                "Projects",
                "WebBrowser",
                "WebBrowserWidget",
                "ToolMenus",
                "GraphEditor",
                "Kismet",
                "KismetCompiler",
                "PropertyEditor",
                "BlueprintGraph",
                "AssetRegistry",
                "ContentBrowserData",
                "DesktopPlatform",
                "SQLiteCore",
                "AnimGraph",
                "EnhancedInput",
                "InputBlueprintNodes",
                "ImageCore",
                "RenderCore",
                "RHI",
                "EditorFramework",
                "MessageLog",
                "ClassViewer",
                "InterchangeEngine",
                "UMG",
                "UMGEditor",
                "AIModule",
                "GameplayTasks",
                "NavigationSystem",
                "AIGraph",
                "BehaviorTreeEditor",
                "AnimationBlueprintLibrary",
                "AnimationModifiers",
                "Persona",
                "AssetTools",
                "EditorScriptingUtilities",
                "AnimationBlueprintEditor",
                "AnimGraphRuntime",
                "MovieScene",
                "MovieSceneTracks",
                "LevelSequenceEditor",
                "EnvironmentQueryEditor",
                "GameplayTags",
                "GameplayTagsEditor",
                "MaterialEditor",
                "AudioEditor",
                "LevelSequence",
                "Landscape",
                "LandscapeEditor",
                "Foliage",
                "Chaos",
                "GeometryCollectionEngine",
                "ChaosSolverEngine",
                "FieldSystemEngine",
                "PhysicsCore",
                "PhysicsUtilities",
                "SkeletalMeshEditor",
                "StructUtils"
			// ... add private dependencies that you statically link with here ...
		}
        );

        // === Optional plugin dependencies (conditional compilation) ===
        // These only compile when the corresponding plugin is enabled in the project.
        // Each gets a preprocessor define so C++ code can #if guard the implementations.

        // Niagara is a standard engine plugin since UE 5.0 â€” always available.
        // Using direct dependency instead of SetupOptionalModule because the
        // file search can fail on installed engine builds due to path/permission issues.
        PrivateDependencyModuleNames.Add("Niagara");
        PrivateDependencyModuleNames.Add("NiagaraCore");
        // NiagaraEditor: required for emitter graph editing â€” UNiagaraScriptSource,
        // UNiagaraGraph, UNiagaraNodeFunctionCall, UNiagaraNodeOutput,
        // FNiagaraStackGraphUtilities, UNiagaraSystemEditorData, UEdGraphSchema_Niagara,
        // and UNiagaraDataInterfaceColorCurve are all in NiagaraEditor.
        // Editor-only â€” gated by WITH_EDITOR in any runtime headers.
        PrivateDependencyModuleNames.Add("NiagaraEditor");
        PublicDefinitions.Add("WITH_NIAGARA=1");
        SetupOptionalModule(Target, "SmartObjectsModule", "WITH_SMART_OBJECTS");
        SetupOptionalModule(Target, "StateTreeModule", "WITH_STATE_TREE");
        SetupOptionalModule(Target, "StateTreeEditorModule", "WITH_STATE_TREE_EDITOR");
        // PropertyBindingUtils â€” UE 5.6+ extracted FPropertyBindingPath /
        // FPropertyBindingBindingCollection out of StateTree into this plugin.
        // StateTree's bind_property path links against it directly.
        SetupOptionalModule(Target, "PropertyBindingUtils", "WITH_PROPERTY_BINDING_UTILS");
        // GameplayAbilities ships as an engine plugin (Engine/Plugins/Runtime/GameplayAbilities/)
        // since UE 4.x. Always present on disk even though it's disabled-by-default in new
        // projects. Same reason as PCG/MetaSound/IKRig below â€” SetupOptionalModule's path
        // search misses Engine/Plugins/Runtime on installed UE builds, so we add the direct
        // dep and set the define manually. User still needs to enable the plugin in .uproject.
        PrivateDependencyModuleNames.Add("GameplayAbilities");
        PublicDefinitions.Add("WITH_GAMEPLAY_ABILITIES=1");
        SetupOptionalModule(Target, "CommonUI", "WITH_COMMON_UI");
        SetupOptionalModule(Target, "ModelViewViewModel", "WITH_MVVM");
        SetupOptionalModule(Target, "ModelViewViewModelBlueprint", "WITH_MVVM_BLUEPRINT");
        // PCG ships as an engine plugin (Engine/Plugins/PCG/) in UE 5.3+ â€” always
        // present on disk even though it's disabled-by-default in new projects.
        // We add it as a direct dependency (not SetupOptionalModule, whose path
        // search misses Engine/Plugins for installed UE builds) so our plugin
        // always links against PCG. The user still needs to enable the plugin
        // in the project's .uproject for the module to load at runtime.
        PrivateDependencyModuleNames.Add("PCG");
        PrivateDependencyModuleNames.Add("PCGEditor");
        PublicDefinitions.Add("WITH_PCG=1");
        PublicDefinitions.Add("WITH_PCG_EDITOR=1");
        // MetaSound ships as an engine plugin (Engine/Plugins/Runtime/Metasound/)
        // in UE 5.0+. The Engine + Frontend modules are runtime; Editor module is
        // editor-only and provides BuildToAsset / FindOrBeginBuilding. Direct deps
        // for the same reason as PCG / Niagara â€” SetupOptionalModule can miss
        // engine plugin paths on installed builds.
        // FlopMetaSoundEditor uses APIs added in UE 5.4 (UMetaSoundBuilderSubsystem::Get,
        // FMetasoundFrontendClassName::Parse/IsValid, the 2-arg FMetasoundFrontendClassName
        // ctor, and the reordered AddNodeByClassName signature). Disable the MetaSound
        // tool surface on 5.3 rather than shimming every one of those APIs individually â€”
        // the file's #if !WITH_METASOUND branches already return a clean "unavailable" error.
        bool bMetaSoundSupported =
            (Target.Version.MajorVersion > 5) ||
            (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 4);
        if (bMetaSoundSupported)
        {
            PrivateDependencyModuleNames.Add("MetasoundEngine");
            PrivateDependencyModuleNames.Add("MetasoundFrontend");
            PrivateDependencyModuleNames.Add("MetasoundEditor");
            PublicDefinitions.Add("WITH_METASOUND=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_METASOUND=0");
        }
        // IK Rig + Retargeter ship as an engine plugin (Engine/Plugins/Animation/IKRig/)
        // since UE 5.0. Always present; user may or may not have the plugin enabled
        // in their .uproject. Editor module is needed for UIKRigController /
        // UIKRetargeterController / FIKRetargetBatchOperation.
        PrivateDependencyModuleNames.Add("IKRig");
        PrivateDependencyModuleNames.Add("IKRigEditor");
        PublicDefinitions.Add("WITH_IKRIG=1");

        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");

        PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
                "UnrealEd"
            }
        );

        PublicDefinitions.AddRange(
            new string[]
            {
                "BPTE_WITH_EDITOR_ONLY=1"
            }
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }

    private void SetupOptionalModule(ReadOnlyTargetRules Target, string ModuleName, string DefineName)
    {
        // Try to add the module directly â€” UBT will resolve it if available.
        // This works for both source builds and installed builds where file
        // path searches may fail due to different directory layouts.
        bool bModuleAvailable = false;
        try
        {
            // Primary approach: search for the Build.cs file in known locations
            string BuildFileName = ModuleName + ".Build.cs";
            string EngineDir = System.IO.Path.GetFullPath(System.IO.Path.Combine(EngineDirectory, ".."));
            string ProjectDir = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "..", "..", "..", ".."));

            string[] SearchRoots = new string[]
            {
                // Installed engine builds (Epic Launcher): plugins live under
                // <UE_x.y>/Engine/Plugins. EngineDirectory IS the Engine dir,
                // so this is the canonical lookup. The other paths below cover
                // source builds, plugin-from-project layouts, and fallbacks.
                System.IO.Path.Combine(EngineDirectory, "Plugins"),
                System.IO.Path.Combine(EngineDirectory, "Source"),
                System.IO.Path.Combine(EngineDir, "Plugins"),
                System.IO.Path.Combine(ProjectDir, "Plugins"),
                System.IO.Path.Combine(ProjectDir, "Source"),
                System.IO.Path.Combine(EngineDir, "Source"),
                System.IO.Path.Combine(EngineDir, "..", "Plugins"),
            };

            foreach (string Root in SearchRoots)
            {
                if (System.IO.Directory.Exists(Root))
                {
                    string[] Found = System.IO.Directory.GetFiles(Root, BuildFileName, System.IO.SearchOption.AllDirectories);
                    if (Found.Length > 0)
                    {
                        bModuleAvailable = true;
                        break;
                    }
                }
            }

            // Fallback for installed builds: check if the module's public headers exist
            // in the Engine/Plugins directory tree (handles Epic launcher installs).
            // EngineDirectory IS the Engine dir, so the correct lookup is
            // <EngineDirectory>/Plugins, not the parent (which has no Plugins
            // folder in installed layouts).
            if (!bModuleAvailable)
            {
                string HeaderName = ModuleName + ".h";
                string[] PluginsRoots = new string[]
                {
                    System.IO.Path.Combine(EngineDirectory, "Plugins"),
                    System.IO.Path.Combine(EngineDir, "Plugins"),
                };
                foreach (string PluginsDir in PluginsRoots)
                {
                    if (System.IO.Directory.Exists(PluginsDir))
                    {
                        string[] HeaderFiles = System.IO.Directory.GetFiles(PluginsDir, HeaderName, System.IO.SearchOption.AllDirectories);
                        if (HeaderFiles.Length > 0)
                        {
                            bModuleAvailable = true;
                            break;
                        }
                    }
                }
            }
        }
        catch
        {
            bModuleAvailable = false;
        }

        if (bModuleAvailable)
        {
            PrivateDependencyModuleNames.Add(ModuleName);
            PublicDefinitions.Add(DefineName + "=1");
        }
        else
        {
            PublicDefinitions.Add(DefineName + "=0");
        }
    }

}

