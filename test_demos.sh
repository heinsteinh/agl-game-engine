#!/bin/bash

# Test all demo applications
# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}🎮 Testing AGL Game Engine Demo Applications${NC}"
echo "=============================================="

# Array of demos to test
demos=(
    "signalslot"
    "dispatchqueue" 
    "renderer"
    "shooter_camera"
    "advanced_renderer"
    "texture"
    "example_logger"
    "benchmark_deltatime"
)

# Build and test each demo
for demo in "${demos[@]}"; do
    echo -e "\n${YELLOW}Building ${demo} demo...${NC}"
    
    # Create clean build directory
    build_dir="sandbox/build_${demo}"
    rm -rf "$build_dir"
    
    # Configure demo
    cd sandbox
    if cmake -B "build_${demo}" -DDEMO_NAME="${demo}" -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1; then
        echo -e "${GREEN}✅ ${demo} configured successfully${NC}"
        
        # Build demo
        if cmake --build "build_${demo}" --parallel > /dev/null 2>&1; then
            echo -e "${GREEN}✅ ${demo} built successfully${NC}"
            
            # Check if executable exists
            exe_name="agl_${demo}_demo"
            if [ -f "build_${demo}/${exe_name}" ]; then
                echo -e "${GREEN}✅ ${demo} executable found: build_${demo}/${exe_name}${NC}"
            else
                echo -e "${RED}❌ ${demo} executable not found${NC}"
            fi
        else
            echo -e "${RED}❌ ${demo} build failed${NC}"
        fi
    else
        echo -e "${RED}❌ ${demo} configuration failed${NC}"
    fi
    
    cd ..
done

echo -e "\n${YELLOW}Demo testing completed!${NC}"
echo -e "\nTo run a specific demo:"
echo -e "${GREEN}./sandbox/build_signalslot/agl_signalslot_demo${NC}"
echo -e "${GREEN}./sandbox/build_dispatchqueue/agl_dispatchqueue_demo${NC}"
echo -e "${GREEN}./sandbox/build_renderer/agl_renderer_demo${NC}"
