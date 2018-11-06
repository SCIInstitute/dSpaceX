import React from 'react';
import { withDSXContext } from '../dsxContext.js';


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

    this.client = this.props.dsxContext.client;
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    let nodes = [];
    for (let i = 0; i < 10; i++) {
      nodes.push({ x:0, y:0 });
    }
    return (
      <svg width="100%" height="100%">
        {
          nodes.map((node, i) => {
            return (
              <circle key={i} cx={0} cy={0} r={4} />
            );
          })
        }
      </svg>
    );
  }
}

export default withDSXContext(GraphD3Window);
