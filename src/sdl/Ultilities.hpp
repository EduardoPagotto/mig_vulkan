#pragma once

struct QueueFamilyIndices {
    int graphicsFamily = -1; // Location of graphics Queue Family

    // check if queue families are valid
    bool isValid() { return graphicsFamily >= 0; }
};
