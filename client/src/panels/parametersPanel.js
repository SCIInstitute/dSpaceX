import React from "react";
import * as d3 from "d3";

/**
 * The Parameters Panel component provides a display of the
 * parameters for the dataset
 */
class ParametersPanel extends React.Component {

    constructor(props) {
        super(props);

        this.client = this.props.client;
    }

    componentDidMount() {
        console.log("Parameter panel mounted");
    }

    render() {
        return <div id="parameterPanelTest"></div>;
    }
}

export default ParametersPanel;