#pragma once

struct QueueFamilyIndices {
    int graphicsFamily = -1;     // Location of graphics Queue Family
    int presentationFamily = -1; // Location of Presentation Queue family

    // check if queue families are valid
    bool isValid() { return graphicsFamily >= 0 && presentationFamily >= 0; }
};
