// Get error details from URL parameters
const urlParams = new URLSearchParams(window.location.search);
const errorCode = urlParams.get('code') || '500';
const errorMessage = urlParams.get('message') || '';

// Error code to details mapping
const errorDetails = {
    '400': {
        icon: '🚫',
        title: 'Bad Request',
        description: 'The server cannot process the request due to client error.'
    },
    '403': {
        icon: '🔒',
        title: 'Forbidden',
        description: 'You don\'t have permission to access this resource.'
    },
    '404': {
        icon: '🔍',
        title: 'Not Found',
        description: 'The requested resource could not be found.'
    },
    '405': {
        icon: '⚙️',
        title: 'Method Not Allowed',
        description: 'The request method is not supported for this resource.'
    },
    '500': {
        icon: '💥',
        title: 'Internal Server Error',
        description: 'The server encountered an unexpected condition.'
    },
    '501': {
        icon: '🛠️',
        title: 'Not Implemented',
        description: 'The server does not support the functionality required.'
    },
    '503': {
        icon: '⏳',
        title: 'Service Unavailable',
        description: 'The server is currently unable to handle the request.'
    }
};

// Set error details
document.getElementById('errorCode').textContent = errorCode;
const details = errorDetails[errorCode] || {
    icon: '⚠️',
    title: 'Error',
    description: errorMessage || 'An unexpected error occurred.'
};

document.getElementById('errorIcon').textContent = details.icon;
document.getElementById('errorTitle').textContent = details.title;
document.getElementById('errorDescription').textContent = details.description;