package fmikit;

import java.util.ArrayList;
import java.util.List;

public class BuildDescription {

    public static class PreprocessorDefinition {
        public String name;
        public String value;
    }

    public static class SourceFileSet {
        public String name;
        public String language;
        public String compiler;
        public String compilerOptions;
        public List<PreprocessorDefinition> preprocessorDefinitions = new ArrayList<PreprocessorDefinition>();
        public List<String> sourceFiles = new ArrayList<String>();
        public List<String> includeDirectories = new ArrayList<String>();
    }

    public static class BuildConfiguration {
        public List<SourceFileSet> sourceFileSets = new ArrayList<SourceFileSet>();
        public String modelIdentifier;
    }

    public List<BuildConfiguration> buildConfigurations = new ArrayList<BuildConfiguration>();

}
