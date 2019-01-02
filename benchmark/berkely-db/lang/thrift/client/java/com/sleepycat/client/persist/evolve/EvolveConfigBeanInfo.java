/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.evolve;

import com.sleepycat.client.util.ConfigBeanInfoBase;

import java.beans.BeanDescriptor;
import java.beans.PropertyDescriptor;

public class EvolveConfigBeanInfo extends ConfigBeanInfoBase {

    @Override
    public BeanDescriptor getBeanDescriptor() {
        return getBdescriptor(EvolveConfig.class);
    }

    @Override
    public PropertyDescriptor[] getPropertyDescriptors() {
        return getPdescriptor(EvolveConfig.class);
    }
}
