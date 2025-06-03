#include "wma/exceptions/WMAException.hpp"

namespace wma {

    // WMAException implementations are all inline in the header
    // This file exists to ensure proper linking and can contain
    // additional exception utilities if needed in the future

    namespace detail {
        
        /**
         * @brief Helper function to format exception messages
         * @param component The component where error occurred
         * @param operation The operation that failed
         * @param details Additional error details
         * @return Formatted error message
         */
        std::string formatExceptionMessage(
            const std::string& component, 
            const std::string& operation, 
            const std::string& details
        ) {
            return component + " Error in " + operation + ": " + details;
        }

        /**
         * @brief Helper to get platform-specific error descriptions
         * @param errorCode Platform-specific error code
         * @return Human-readable error description
         */
        std::string getPlatformErrorString(int errorCode) {
            // This could be expanded to provide platform-specific error translations
            return "Error code: " + std::to_string(errorCode);
        }

    } // namespace detail

} // namespace wma
