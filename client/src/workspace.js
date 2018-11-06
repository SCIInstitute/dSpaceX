import React from 'react';

/**
 * The workspace component encapsulates the space containing subwindows.
 * It does not include the navigation bar or drawer components.
 */
class Workspace extends React.Component {
  /**
   * Workspace class constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <div className={this.props.className}>
        {this.props.children}
      </div>
    );
  }
}

export default Workspace;
