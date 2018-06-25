import React from 'react';


/**
 * A D3 Window Component for rendering Graphs.
 */
class GraphD3Window extends React.Component {
  /**
   * GraphD3Window constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.client = this.props.client;
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <svg width="100%" height="100%">

      </svg>
    );
  }
}

export default GraphD3Window;
