import React from 'react';

export const DSXContext = React.createContext();
export const DSXProvider = DSXContext.Provider;

/**
 * Higher Order Component to Provide DSX Context as prop to base component.
 * @param {object} Component
 * @return {HTML}
 */
export function withDSXContext(Component) {
  return function ContextAwareComponent(props) {
    return (
      <DSXContext.Consumer>
        {
          (context) => <Component {...props} dsxContext={context} />
        }
      </DSXContext.Consumer>
    );
  };
}
