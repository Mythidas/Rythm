#include "core/application.h"
#include "core/logger.h"

int main() {
    try {
        rm::Application application({});
        application.Run();
    } catch (const std::exception& e) {
        RM_LOG_ERROR("Fatal Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
